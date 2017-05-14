#include "sitelogic.h"

#include "core/tickpoke.h"
#include "core/eventreceiver.h"
#include "core/types.h"
#include "sitemanager.h"
#include "ftpconn.h"
#include "filelist.h"
#include "siterace.h"
#include "site.h"
#include "sitelogicmanager.h"
#include "globalcontext.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"
#include "sitelogicrequest.h"
#include "sitelogicrequestready.h"
#include "uibase.h"
#include "rawbuffer.h"
#include "engine.h"
#include "connstatetracker.h"
#include "transfermonitor.h"
#include "delayedcommand.h"
#include "potentiallistelement.h"
#include "eventlog.h"
#include "recursivecommandlogic.h"
#include "transfermanager.h"
#include "localstorage.h"
#include "transferjob.h"
#include "commandowner.h"
#include "util.h"
#include "race.h"

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150
//maximum number of dir refreshes in a row in the same race
#define MAXCHECKSROW 5

// maximum number of ready requests available to be checked out
#define MAXREQUESTREADYQUEUE 10

#define TICKINTERVAL 50
#define FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH 2000
#define FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH_XDUPE 5000

enum RequestType {
  REQ_FILELIST,
  REQ_RAW,
  REQ_WIPE_RECURSIVE,
  REQ_WIPE,
  REQ_DEL_RECURSIVE,
  REQ_DEL_OWN,
  REQ_DEL,
  REQ_NUKE,
  REQ_IDLE
};

enum Exists {
  EXISTS_NO,
  EXISTS_YES,
  EXISTS_FAILED
};

SiteLogic::SiteLogic(const std::string & sitename) :
  site(global->getSiteManager()->getSite(sitename)),
  rawcommandrawbuf(new RawBuffer(site->getName())),
  aggregatedrawbuf(new RawBuffer(site->getName())),
  maxslotsup(site->getMaxUp()),
  maxslotsdn(site->getMaxDown()),
  slotsdn(maxslotsdn),
  slotsup(maxslotsup),
  available(0),
  ptrack(new PotentialTracker(slotsdn)),
  loggedin(0),
  requestidcounter(0),
  poke(false),
  currtime(0)
{
  global->getTickPoke()->startPoke(this, "SiteLogic", TICKINTERVAL, 0);

  for (unsigned int i = 0; i < site->getMaxLogins(); i++) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, i));
  }
}

SiteLogic::~SiteLogic() {
  global->getTickPoke()->stopPoke(this, 0);
  delete ptrack;
  for (unsigned int i = 0; i < conns.size(); i++) {
    delete conns[i];
  }
  for (unsigned int i = 0; i < races.size(); i++) {
    delete races[i];
  }
  delete rawcommandrawbuf;
  delete aggregatedrawbuf;
}

void SiteLogic::activateAll() {
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (!conns[i]->isConnected()) {
      connstatetracker[i].resetIdleTime();
      conns[i]->login();
    }
    else {
      handleConnection(i, false);
    }
  }
}

SiteRace * SiteLogic::addRace(Pointer<Race> & enginerace, const std::string & section, const std::string & release) {
  SiteRace * race = new SiteRace(enginerace, site->getName(), site->getSectionPath(section), release, site->getUser(), site->getSkipList());
  races.push_back(race);
  activateAll();
  return race;
}

void SiteLogic::addTransferJob(Pointer<TransferJob> tj) {
  transferjobs.push_back(tj);
  activateOne();
}

void SiteLogic::tick(int message) {
  currtime += TICKINTERVAL;
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(TICKINTERVAL);
    if (connstatetracker[i].getCommand().isReleased()) {
      util::assert(!connstatetracker[i].isLocked() && !conns[i]->isProcessing());
      DelayedCommand eventcommand = connstatetracker[i].getCommand();
      connstatetracker[i].getCommand().reset();
      connstatetracker[i].use();
      std::string event = eventcommand.getCommand();
      if (event == "refreshchangepath") {
        SiteRace * race = (SiteRace *) eventcommand.getArg();
        refreshChangePath(i, race, true);
      }
      else if (event == "handle") {
        handleConnection(i, false);
      }
      else if (event == "userpass") {
        conns[i]->doUSER(false);
      }
      else if (event == "reconnect") {
        conns[i]->reconnect();
      }
      else if (event == "quit") {
        disconnectConn(i);
      }
    }
  }
}

void SiteLogic::connectFailed(int id) {
  connstatetracker[id].setDisconnected();
}

void SiteLogic::userDenied(int id) {
  disconnectConn(id);
}

void SiteLogic::userDeniedSiteFull(int id) {
  connstatetracker[id].delayedCommand("reconnect", SLEEPDELAY, NULL, true);
}

void SiteLogic::userDeniedSimultaneousLogins(int id) {
  conns[id]->doUSER(true);
}

void SiteLogic::loginKillFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::passDenied(int id) {
  disconnectConn(id);
}

void SiteLogic::TLSFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::listRefreshed(int id) {
  connstatetracker[id].resetIdleTime();
  FileList * fl = conns[id]->currentFileList();
  fl->setRefreshedTime(currtime);
  CommandOwner * currentco = conns[id]->currentCommandOwner();
  if (currentco != NULL) {
    currentco->fileListUpdated(this, conns[id]->currentFileList());
  }
  if (connstatetracker[id].getRecursiveLogic()->isActive()) {
    handleRecursiveLogic(id, conns[id]->currentFileList());
    return;
  }
  if (connstatetracker[id].hasRequest()) {
    const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
    FileList * filelist = conns[id]->currentFileList();
    if (request->requestType() == REQ_FILELIST &&
        request->requestData() == filelist->getPath().toString())
    {
      setRequestReady(id, filelist, true);
    }
  }
  if (currentco != NULL) {
    if (currentco->classType() == COMMANDOWNER_SITERACE) {
      SiteRace * sr = (SiteRace *)currentco;
      global->getEngine()->raceFileListRefreshed(this, sr);
    }
  }
  handleConnection(id, true);
}

bool SiteLogic::setPathExists(int id, int exists, bool refreshtime) {
  FileList * fl = conns[id]->currentFileList();
  if (fl && (fl->getState() == FILELIST_UNKNOWN || (fl->getState() == FILELIST_NONEXISTENT && exists != EXISTS_NO) ||
      (fl->getState() == FILELIST_FAILED && exists == EXISTS_YES)))
  {
    switch (exists) {
      case EXISTS_YES:
        fl->setExists();
        break;
      case EXISTS_NO:
        fl->setNonExistent();
        break;
      case EXISTS_FAILED:
        fl->setFailed();
        break;
    }
    if (refreshtime) {
      fl->setRefreshedTime(currtime);
    }
    CommandOwner * currentco = conns[id]->currentCommandOwner();
    if (currentco) {
      currentco->fileListUpdated(this, fl);
    }
    return true;
  }
  return false;
}

bool SiteLogic::handleCommandDelete(int id, bool success) {
  if (connstatetracker[id].hasRequest()) {
    int type = connstatetracker[id].getRequest()->requestType();
    if (type == REQ_DEL) {
      setRequestReady(id, NULL, success);
    }
    else if (type == REQ_DEL_RECURSIVE || type == REQ_DEL_OWN) {
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return true;
      }
      else {
        setRequestReady(id, NULL, success);
      }
    }
  }
  return false;
}

bool SiteLogic::makeTargetDirectory(int id, bool includinglast, CommandOwner * co) {
  Path trypath = conns[id]->getMKDCWDTargetSection();
  std::list<std::string> subdirs = conns[id]->getMKDSubdirs();
  while (co && subdirs.size() > (includinglast ? 0 : 1)) {
    trypath = trypath / subdirs.front();
    subdirs.pop_front();
    FileList * fl = co->getFileListForFullPath(this, trypath);
    if (fl) {
      if ((fl->getState() == FILELIST_UNKNOWN || fl->getState() == FILELIST_NONEXISTENT)) {
        conns[id]->doMKD(fl, co);
        return true;
      }
    }
    else {
      conns[id]->doMKD(trypath, co);
      return true;
    }
  }
  return false;
}

void SiteLogic::commandSuccess(int id) {
  connstatetracker[id].resetIdleTime();
  int state = conns[id]->getState();
  std::list<SiteLogicRequest>::iterator it;
  switch (state) {
    case STATE_LOGIN:
      loggedin++;
      available++;
      connstatetracker[id].setLoggedIn();
      break;
    case STATE_PBSZ:
    case STATE_PROT_P:
    case STATE_PROT_C:
    case STATE_SSCN_ON:
    case STATE_SSCN_OFF:
      break;
    case STATE_PORT:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->activeReady();
        return;
      }
      break;
    case STATE_CWD: {
      setPathExists(id, EXISTS_YES, false);
      if (conns[id]->hasMKDCWDTarget()) {
        if (conns[id]->getCurrentPath() == conns[id]->getMKDCWDTargetSection() / conns[id]->getMKDCWDTargetPath()) {
          conns[id]->finishMKDCWDTarget();
        }
      }
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_FILELIST) {
          getFileListConn(id);
          return;
        }
        else if (request->requestType() == REQ_RAW) {
          conns[id]->doRaw(request->requestData2());
          return;
        }
      }
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      break;
    }
    case STATE_MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        setPathExists(id, EXISTS_YES, true);
        if (targetpath == targetcwdsect / targetcwdpath) {
          conns[id]->finishMKDCWDTarget();
          conns[id]->doCWD(conns[id]->currentFileList(), currentco);
          return;
        }
        if (makeTargetDirectory(id, true, currentco)) {
          return;
        }
      }
      break;
    case STATE_PRET_RETR:
    case STATE_PRET_STOR:
      if (connstatetracker[id].transferInitialized()) {
        if (connstatetracker[id].getTransferPassive()) {
          passiveModeCommand(id);
          return;
        }
      }
      else {
        global->getEventLog()->log("SiteLogic", "BUG: Trying to start an active transfer after PRET. Why oh why?");
      }
      break;
    case STATE_RETR: // RETR started
      if (connstatetracker[id].transferInitialized()) {
        if (!connstatetracker[id].getTransferPassive()) {
          connstatetracker[id].getTransferMonitor()->activeStarted();
        }
      }
      return;
    case STATE_RETR_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->sourceComplete();
        transferComplete(id, true);
        connstatetracker[id].finishTransfer();
        global->getEngine()->raceActionRequest();
        if (conns[id]->isProcessing() || connstatetracker[id].isTransferLocked()) {
          return;
        }
      }
      else {
        global->getEventLog()->log("SiteLogic", "BUG: returned successfully from RETR without having a transfer. Shouldn't happen!");
      }
      break;
    case STATE_STOR: // STOR started
      if (connstatetracker[id].transferInitialized()) {
        if (!connstatetracker[id].getTransferPassive()) {
          connstatetracker[id].getTransferMonitor()->activeStarted();
        }
      }
      return;
    case STATE_STOR_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->targetComplete();
        transferComplete(id, false);
        FileList * fl = connstatetracker[id].getTransferFileList();
        connstatetracker[id].finishTransfer();
        int filelistreusetime = currtime - fl->getRefreshedTime();
        if (filelistreusetime < FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH ||
            (site->useXDUPE() && filelistreusetime < FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH_XDUPE))
        {
          global->getEngine()->raceActionRequest();
          if (conns[id]->isProcessing() || connstatetracker[id].isTransferLocked()) {
            return;
          }
        }
      }
      else {
        global->getEventLog()->log("SiteLogic", "BUG: returned successfully from STOR without having a transfer. Shouldn't happen!");
      }
      break;
    case STATE_ABOR:
      break;
    case STATE_PASV_ABORT:
      break;
    case STATE_WIPE:
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_WIPE_RECURSIVE ||
            request->requestType() == REQ_WIPE)
        {
          setRequestReady(id, NULL, true);
        }
      }
      break;
    case STATE_DELE:
    case STATE_RMD:
      if (handleCommandDelete(id, true)) {
        return;
      }
      break;
    case STATE_NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_NUKE) {
          setRequestReady(id, NULL, true);
        }
      }
      break;
    case STATE_LIST:
      if (!connstatetracker[id].getTransferPassive()) {
        connstatetracker[id].getTransferMonitor()->activeStarted();
      }
      return;
    case STATE_PRET_LIST:
      if (connstatetracker[id].transferInitialized()) {
        if (connstatetracker[id].getTransferPassive()) {
          conns[id]->doPASV();
          return;
        }
      }
      break;
    case STATE_LIST_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        TransferMonitor * tm = connstatetracker[id].getTransferMonitor();
        connstatetracker[id].finishTransfer();
        tm->sourceComplete();
      }
      handleConnection(id, true);
      return;
  }
  handleConnection(id);
}

void SiteLogic::commandFail(int id) {
  commandFail(id, FAIL_UNDEFINED);
}

void SiteLogic::commandFail(int id, int failuretype) {
  connstatetracker[id].resetIdleTime();
  int state = conns[id]->getState();
  switch (state) {
    case STATE_PBSZ:
    case STATE_PROT_P:
    case STATE_PROT_C:
    case STATE_CPSV:
    case STATE_PASV:
    case STATE_PORT:
      if (connstatetracker[id].transferInitialized()) {
        handleTransferFail(id, TM_ERR_OTHER);
        return;
      }
      else {
        handleConnection(id);
        return;
      }
      break;
    case STATE_CWD: {
      bool filelistupdated = false;
      if (setPathExists(id, EXISTS_NO, true)) {
        filelistupdated = true;
      }
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        connstatetracker[id].getRecursiveLogic()->failedCwd();
        if (connstatetracker[id].getRecursiveLogic()->isActive()) {
          handleRecursiveLogic(id);
        }
        else {
          handleConnection(id);
        }
        return;
      }
      else if (conns[id]->hasMKDCWDTarget()) {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        if (!site->getAllowUpload() ||
            (currentco != NULL && currentco->classType() == COMMANDOWNER_SITERACE &&
             site->isAffiliated(((SiteRace *)currentco)->getGroup()))) {
          conns[id]->finishMKDCWDTarget();
        }
        else {
          conns[id]->doMKD(conns[id]->currentFileList(), currentco);
          return;
        }
      }
      else if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_FILELIST ||
            connstatetracker[id].getRequest()->requestType() == REQ_RAW)
        {
          setRequestReady(id, NULL, false);
          handleConnection(id);
          return;
        }
      }
      if (filelistupdated) {
        global->getEngine()->filelistUpdated();
        handleConnection(id);
        return;
      }
      handleFail(id);
      return;
    }
    case STATE_MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        if (targetpath == targetcwdsect / targetcwdpath && makeTargetDirectory(id, false, currentco)) {
          return;
        }
        conns[id]->finishMKDCWDTarget();
        setPathExists(id, EXISTS_FAILED, true);
        conns[id]->doCWD(conns[id]->currentFileList(), currentco);
        return;
      }
      else {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        if (currentco != NULL && currentco->classType() == COMMANDOWNER_TRANSFERJOB) {
          handleConnection(id);
          return;
        }
      }
      break;
    case STATE_PRET_RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_PRET);
      return;
    case STATE_PRET_STOR:
      if (failuretype == FAIL_DUPE) {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_DUPE);
      }
      else {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_PRET);
      }
      return;
    case STATE_RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR);
      return;
    case STATE_RETR_COMPLETE:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR_COMPLETE);
      return;
    case STATE_STOR:
      if (failuretype == FAIL_DUPE) {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_DUPE);
      }
      else {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_RETRSTOR);
      }
      return;
    case STATE_STOR_COMPLETE:
      handleTransferFail(id, CST_UPLOAD, TM_ERR_RETRSTOR_COMPLETE);
      return;
    case STATE_WIPE:
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_WIPE_RECURSIVE ||
            request->requestType() == REQ_WIPE)
        {
          setRequestReady(id, NULL, false);
        }
      }
      handleConnection(id);
      return;
    case STATE_DELE:
    case STATE_RMD:
      if (!handleCommandDelete(id, false)) {
        handleConnection(id);
      }
      return;
    case STATE_NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_NUKE) {
          setRequestReady(id, NULL, false);
        }
      }
      handleConnection(id);
      return;
    case STATE_LIST:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR);
      return;
    case STATE_PRET_LIST:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_PRET);
      return;
    case STATE_LIST_COMPLETE:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR_COMPLETE);
      return;
  }
  // default handling: reconnect
  disconnected(id);
  conns[id]->reconnect();
}

void SiteLogic::checkFailListRequest(int id) {
  if (connstatetracker[id].hasRequest()) {
    if (connstatetracker[id].getRequest()->requestType() == REQ_FILELIST) {
      setRequestReady(id, NULL, false);
    }
  }
}

void SiteLogic::handleFail(int id) {
  if (connstatetracker[id].isListOrTransferLocked()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return;
  }
  if (connstatetracker[id].isLocked()) {
    handleConnection(id);
    return;
  }
  connstatetracker[id].delayedCommand("handle", SLEEPDELAY * 6);
}

void SiteLogic::handleTransferFail(int id, int err) {
  if (connstatetracker[id].isListOrTransferLocked()) {
    handleTransferFail(id, connstatetracker[id].getTransferType(), err);
  }
  else {
    global->getEventLog()->log("SiteLogic", "BUG: Returned failed transfer (code " +
        util::int2Str(err) + ") without having a transfer, shouldn't happen!");
  }
}

void SiteLogic::handleTransferFail(int id, int type, int err) {
  bool passive = connstatetracker[id].getTransferPassive();
  if (connstatetracker[id].isListOrTransferLocked()) {
    reportTransferErrorAndFinish(id, type, err);
  }
  else {
    switch (type) {
      case CST_DOWNLOAD:
        global->getEventLog()->log("SiteLogic", "BUG: Returned failed download (code " +
            util::int2Str(err) + ") without having a transfer, shouldn't happen!");
        break;
      case CST_UPLOAD:
        global->getEventLog()->log("SiteLogic", "BUG: Returned failed upload (code " +
            util::int2Str(err) + ") without having a transfer, shouldn't happen!");
        break;
      case CST_LIST:
        global->getEventLog()->log("SiteLogic", "BUG: Returned failed LIST (code " +
            util::int2Str(err) + ") without having a transfer, shouldn't happen!");
        break;
    }
  }
  if (connstatetracker[id].transferInitialized() && passive &&
      (err == TM_ERR_RETRSTOR || err == TM_ERR_DUPE))
  {
    conns[id]->abortTransferPASV();
  }
  else if (err == TM_ERR_OTHER && !connstatetracker[id].isLocked()) {
    connstatetracker[id].delayedCommand("handle", SLEEPDELAY * 6);
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::reportTransferErrorAndFinish(int id, int err) {
  reportTransferErrorAndFinish(id, connstatetracker[id].getTransferType(), err);
}

void SiteLogic::reportTransferErrorAndFinish(int id, int type, int err) {
  if (!connstatetracker[id].getTransferAborted()) {
    switch (type) {
      case CST_DOWNLOAD:
        connstatetracker[id].getTransferMonitor()->sourceError((TransferError)err);
        transferComplete(id, true);
        break;
      case CST_LIST:
        connstatetracker[id].getTransferMonitor()->sourceError((TransferError)err);
        break;
      case CST_UPLOAD:
        connstatetracker[id].getTransferMonitor()->targetError((TransferError)err);
        transferComplete(id, false);
        break;
    }
  }
  connstatetracker[id].finishTransfer();
}

void SiteLogic::gotPath(int id, const std::string & path) {
  connstatetracker[id].resetIdleTime();
}

void SiteLogic::rawCommandResultRetrieved(int id, const std::string & result) {
  connstatetracker[id].resetIdleTime();
  rawcommandrawbuf->write(result);
  if (connstatetracker[id].hasRequest()) {
    if (connstatetracker[id].getRequest()->requestType() == REQ_RAW) {
      std::string * data = new std::string(result);
      setRequestReady(id, data, true);
    }
  }
  handleConnection(id, true);
}

void SiteLogic::gotPassiveAddress(int id, const std::string & host, int port) {
  connstatetracker[id].resetIdleTime();
  if (connstatetracker[id].transferInitialized()) {
    connstatetracker[id].getTransferMonitor()->passiveReady(host, port);
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::timedout(int id) {

}

void SiteLogic::disconnected(int id) {
  cleanupConnection(id);
  if (connstatetracker[id].isLoggedIn()) {
    loggedin--;
    available--;
  }
  connstatetracker[id].setDisconnected();
}

void SiteLogic::activateOne() {
  if (loggedin == 0) {
    connstatetracker[0].resetIdleTime();
    conns[0]->login();
  }
  else {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
          !conns[i]->isProcessing())
      {
        handleConnection(i, false);
        return;
      }
    }
    if (loggedin < conns.size()) {
      for (unsigned int i = 0; i < conns.size(); i++) {
        if (!conns[i]->isConnected()) {
          connstatetracker[i].resetIdleTime();
          conns[i]->login();
          break;
        }
      }
    }
  }
}

void SiteLogic::haveConnectedActivate(unsigned int connected) {
  if (loggedin < connected) {
    int lefttoconnect = connected;
    for (unsigned int i = 0; i < conns.size() && lefttoconnect > 0; i++) {
      if (!conns[i]->isConnected()) {
        connstatetracker[i].resetIdleTime();
        conns[i]->login();
        lefttoconnect--;
      }
    }
  }
  else {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
          !conns[i]->isProcessing())
      {
        handleConnection(i, false);
        break;
      }
    }
  }
}

bool SiteLogic::handlePreTransfer(int id) {
  if (connstatetracker[id].getTransferMonitor()->willFail()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return false;
  }
  FileList * fl = connstatetracker[id].getTransferFileList();
  const Path & transferpath = fl->getPath();
  CommandOwner * co = connstatetracker[id].getCommandOwner();
  if (conns[id]->getCurrentPath() != transferpath) {
    if (connstatetracker[id].getTransferType() == CST_UPLOAD &&
        fl->getState() == FILELIST_NONEXISTENT)
    {
      std::pair<Path, Path> pathparts = site->splitPathInSectionAndSubpath(transferpath);
      conns[id]->setMKDCWDTarget(pathparts.first, pathparts.second);
      return makeTargetDirectory(id, true, co);
    }
    conns[id]->doCWD(fl, co);
    return true;
  }
  return false;
}

void SiteLogic::handleConnection(int id) {
  handleConnection(id, false);
}

void SiteLogic::handleConnection(int id, bool backfromrefresh) {
  if (conns[id]->isProcessing()) {
    return;
  }
  if (connstatetracker[id].isLocked()) {
    if (connstatetracker[id].hasTransfer()) {
      connstatetracker[id].resetIdleTime();
      initTransfer(id);
      return;
    }
    if (connstatetracker[id].isTransferLocked()) {
      handlePreTransfer(id);
    }
    return;
  }
  if (requests.size() > 0) {
    if (handleRequest(id)) {
      return;
    }
  }
  std::list<Pointer<TransferJob> > targetjobs;
  for (std::list<Pointer<TransferJob> >::const_iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    Pointer<TransferJob> tj = *it;
    if (!tj->isDone()) {
      if (tj->wantsList(this)) {
        FileList * fl = tj->getListTarget(this);
        const Path & path = fl->getPath();
        connstatetracker[id].use();
        if (path != conns[id]->getCurrentPath()) {
          conns[id]->doCWD(fl, tj.get());
          return;
        }
        getFileListConn(id, tj.get(), fl);
        return;
      }
      if (!tj->isInitialized()) {
        targetjobs.push_front(tj);
        continue;
      }
      int type = tj->getType();
      if (((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) &&
            tj->getSrc()->getCurrDown() < tj->maxSlots()) ||
          (type == TRANSFERJOB_UPLOAD && getCurrUp() < tj->maxSlots()))
      {
        targetjobs.push_back(tj);
        continue;
      }
    }
  }
  if (targetjobs.size()) {
    connstatetracker[id].delayedCommand("handle", SLEEPDELAY);
  }
  for (std::list<Pointer<TransferJob> >::iterator it = targetjobs.begin(); it != targetjobs.end(); it++) {
    if (global->getEngine()->transferJobActionRequest(this, *it)) {
      return;
    }
  }
  if (targetjobs.size()) {
    connstatetracker[id].use();
  }
  SiteRace * race = NULL;
  bool refresh = false;
  SiteRace * lastchecked = connstatetracker[id].lastChecked();
  if (lastchecked && !lastchecked->isDone() && lastchecked->anyFileListNotNonExistent() &&
      connstatetracker[id].checkCount() < MAXCHECKSROW)
  {
    race = lastchecked;
    refresh = true;
    connstatetracker[id].check(race);
  }
  else {
    for (unsigned int i = 0; i < races.size(); i++) {
      if (!races[i]->isDone() && !wasRecentlyListed(races[i]) && races[i]->anyFileListNotNonExistent()) {
        race = races[i];
        break;
      }
    }
    if (race == NULL) {
      for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
        if (!(*it)->isDone() && (*it)->anyFileListNotNonExistent()) {
          race = *it;
          break;
        }
      }
    }
    if (race != NULL) {
      refresh = true;
      connstatetracker[id].check(race);
      addRecentList(race);
    }
  }
  if (race == NULL) {
    for (unsigned int i = 0; i < races.size(); i++) {
      if (!races[i]->isGlobalDone() && !races[i]->isAborted() && races[i]->anyFileListNotNonExistent()) {
        race = races[i];
        break;
      }
    }
  }
  if (race == NULL) {
    for (unsigned int i = 0; i < races.size(); i++) {
      if (!races[i]->isGlobalDone() && !races[i]->isAborted()) {
        race = races[i];
        break;
      }
    }
  }
  if (race != NULL) {
    const Path & currentpath = conns[id]->getCurrentPath();
    const Path & racepath = race->getPath();
    bool goodpath = currentpath == racepath || // same path
                    currentpath.contains(racepath); // same path, currently in subdir
    if (goodpath && !refresh) {
      return;
    }
    if (backfromrefresh) {
      connstatetracker[id].delayedCommand("refreshchangepath", SLEEPDELAY, (void *) race);
      return;
    }
    connstatetracker[id].use();

    if (goodpath) {
      Path subpath = currentpath - racepath;
      FileList * fl = race->getFileListForPath(subpath.toString());
      if (fl == NULL) {
        if (!race->addSubDirectory(subpath.toString(), true)) {
          refreshChangePath(id, race, refresh);
          return;
        }
        fl = race->getFileListForPath(subpath.toString());
      }
      getFileListConn(id, race, fl);
      return;
    }
    else {
      refreshChangePath(id, race, refresh);
    }
  }
  else {
    if (site->getMaxIdleTime() && !connstatetracker[id].getCommand().isActive()) {
      connstatetracker[id].delayedCommand("quit", site->getMaxIdleTime() * 1000);
    }
  }
}

bool SiteLogic::handleRequest(int id) {
  std::list<SiteLogicRequest>::iterator it;
  for (it = requests.begin(); it != requests.end(); it++) {
    if (it->connId() == id) {
      break;
    }
  }
  if (it == requests.end()) {
    for (it = requests.begin(); it != requests.end(); it++) {
      if (it->connId() == -1) {
        it->setConnId(id);
        break;
      }
    }
  }
  if (it == requests.end()) {
    return false;
  }
  connstatetracker[id].use();
  SiteLogicRequest request = *it;
  requests.erase(it);
  connstatetracker[id].setRequest(request);
  available--;

  switch (request.requestType()) {
    case REQ_FILELIST: { // filelist
      Path targetpath = request.requestData();
      if (conns[id]->getCurrentPath() == targetpath) {
        getFileListConn(id);
      }
      else {
        conns[id]->doCWD(targetpath);
      }
      break;
    }
    case REQ_RAW: { // raw command
      Path targetpath = request.requestData();
      if (conns[id]->getCurrentPath() != targetpath) {
        conns[id]->doCWD(targetpath);
      }
      else {
        rawcommandrawbuf->writeLine(request.requestData2());
        conns[id]->doRaw(request.requestData2());
      }
      break;
    }
    case REQ_WIPE_RECURSIVE: // recursive wipe
      conns[id]->doWipe(request.requestData(), true);
      break;
    case REQ_WIPE: // wipe
      conns[id]->doWipe(request.requestData(), false);
      break;
    case REQ_DEL_RECURSIVE: { // recursive delete
      std::string targetpath = request.requestData();
      connstatetracker[id].getRecursiveLogic()->initialize(RCL_DELETE, targetpath, site->getUser());
      handleRecursiveLogic(id);
      break;
    }
    case REQ_DEL_OWN: { // recursive delete of own files
      std::string targetpath = request.requestData();
      connstatetracker[id].getRecursiveLogic()->initialize(RCL_DELETEOWN, targetpath, site->getUser());
      handleRecursiveLogic(id);
      break;
    }
    case REQ_DEL: // delete
      conns[id]->doDELE(request.requestData());
      break;
    case REQ_NUKE: // nuke
      conns[id]->doNuke(request.requestData(), request.requestData3(), request.requestData2());
      break;
    case REQ_IDLE: // idle
      setRequestReady(id, NULL, true);
      handleConnection(id);
      if (!conns[id]->isProcessing()) {
        connstatetracker[id].delayedCommand("quit", request.requestData3() * 1000);
      }
      break;
  }
  return true;
}

void SiteLogic::handleRecursiveLogic(int id) {
  handleRecursiveLogic(id, NULL);
}

void SiteLogic::handleRecursiveLogic(int id, FileList * fl) {
  Path actiontarget;
  if (fl != NULL) {
    connstatetracker[id].getRecursiveLogic()->addFileList(fl);
  }
  switch (connstatetracker[id].getRecursiveLogic()->getAction(conns[id]->getCurrentPath(), actiontarget)) {
    case RCL_ACTION_LIST:
      getFileListConn(id, true);
      break;
    case RCL_ACTION_CWD:
      conns[id]->doCWD(actiontarget);
      break;
    case RCL_ACTION_DELETE:
      conns[id]->doDELE(actiontarget);
      break;
    case RCL_ACTION_DELDIR:
      conns[id]->doRMD(actiontarget);
      break;
    case RCL_ACTION_NOOP:
      if (connstatetracker[id].hasRequest()) {
        int type = connstatetracker[id].getRequest()->requestType();
        if (type == REQ_DEL || type == REQ_DEL_RECURSIVE || type == REQ_DEL_OWN) {
          setRequestReady(id, NULL, true);
        }
      }
      handleConnection(id);
      break;
  }
}

void SiteLogic::addRecentList(SiteRace * sr) {
  for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  recentlylistedraces.push_back(sr);
}

bool SiteLogic::wasRecentlyListed(SiteRace * sr) const {
  for (std::list<SiteRace *>::const_iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      return true;
    }
  }
  return false;
}

void SiteLogic::refreshChangePath(int id, SiteRace * race, bool refresh) {
  const Path & currentpath = conns[id]->getCurrentPath();
  std::string subpath = race->getRelevantSubPath();
  Path targetpath = race->getPath() / subpath;
  FileList * fl = race->getFileListForFullPath(this, targetpath);
  if (targetpath != currentpath) {
    conns[id]->doCWD(fl, race);
  }
  else {
    if (refresh) {
      getFileListConn(id, race, fl);
    }
  }
}

void SiteLogic::initTransfer(int id) {
  if (conns[id]->isProcessing()) {
    return;
  }
  if (connstatetracker[id].getTransferMonitor()->willFail()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return;
  }
  int transfertype = connstatetracker[id].getTransferType();
  if (!connstatetracker[id].transferInitialized()) {
    connstatetracker[id].initializeTransfer();
  }
  bool transferssl = connstatetracker[id].getTransferSSL();
  if (transfertype != CST_LIST) {
    if (handlePreTransfer(id)) {
      return;
    }
  }
  if (transferssl && conns[id]->getProtectedMode() != PROT_P) {
    conns[id]->doPROTP();
    return;
  }
  else if (!transferssl && conns[id]->getProtectedMode() != PROT_C) {
    if (site->SSL() || conns[id]->getProtectedMode() == PROT_P) {
      conns[id]->doPROTC();
      return;
    }
  }
  if (!connstatetracker[id].getTransferPassive()) { // active
    if (connstatetracker[id].getTransferMonitor()->getStatus() ==
        TM_STATUS_AWAITING_ACTIVE) {
      if (conns[id]->getSSCNMode()) {
        conns[id]->doSSCN(false);
        return;
      }
      conns[id]->doPORT(connstatetracker[id].getTransferHost(),
                        connstatetracker[id].getTransferPort());
    }
  }
  else { // passive
    if (connstatetracker[id].getTransferMonitor()->getStatus() ==
        TM_STATUS_AWAITING_PASSIVE) {
      if (connstatetracker[id].getTransferFXP() && transferssl &&
          !site->supportsCPSV()) {
        if (!conns[id]->getSSCNMode()) {
          conns[id]->doSSCN(true);
          return;
        }
      }
      else if (conns[id]->getSSCNMode()) {
        conns[id]->doSSCN(false);
        return;
      }
      if (site->needsPRET()) {
        switch (transfertype) {
          case CST_DOWNLOAD:
            conns[id]->doPRETRETR(connstatetracker[id].getTransferFile());
            break;
          case CST_UPLOAD:
            conns[id]->doPRETSTOR(connstatetracker[id].getTransferFile());
            break;
          case CST_LIST:
            conns[id]->doPRETLIST();
            break;
        }
      }
      else {
        passiveModeCommand(id);
      }
    }
  }
}

int SiteLogic::requestFileList(const Path & path) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_FILELIST, path.toString(), true));
  activateOne();
  return requestid;
}

int SiteLogic::requestRawCommand(const std::string & command) {
  return requestRawCommand(site->getBasePath(), command, false);
}

int SiteLogic::requestRawCommand(const Path & path, const std::string & command, bool care) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_RAW, path.toString(), command, care));
  activateOne();
  return requestid;
}

int SiteLogic::requestWipe(const Path & path, bool recursive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE_RECURSIVE, path.toString(), true));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE, path.toString(), true));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestDelete(const Path & path, bool recursive, bool interactive, bool allfiles) {
  int requestid = requestidcounter++;
  if (recursive) {
    int req = allfiles ? REQ_DEL_RECURSIVE : REQ_DEL_OWN;
    requests.push_back(SiteLogicRequest(requestid, req, path.toString(), interactive));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_DEL, path.toString(), interactive));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestNuke(const Path & path, int multiplier, const std::string & reason) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_NUKE, path.toString(), reason, multiplier, true));
  activateOne();
  return requestid;
}

int SiteLogic::requestOneIdle() {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_IDLE, site->getMaxIdleTime(), true));
  activateOne();
  return requestid;
}

int SiteLogic::requestAllIdle(int idletime) {
  if (!idletime) {
    idletime = site->getMaxIdleTime();
  }
  int requestid;
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    requestid = requestidcounter++;
    SiteLogicRequest request(requestid, REQ_IDLE, idletime, false);
    request.setConnId(i);
    requests.push_back(request);
  }
  activateAll();
  return requestid;
}

bool SiteLogic::requestReady(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return true;
    }
  }
  return false;
}

void SiteLogic::abortRace(unsigned int id) {
  SiteRace * delrace = NULL;
  for (std::vector<SiteRace *>::iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getId() == id) {
      delrace = *it;
      delrace->abort();
      break;
    }
  }
  if (delrace != NULL) {
    for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
      if ((*it) == delrace) {
        recentlylistedraces.erase(it);
        break;
      }
    }
  }
}

FileList * SiteLogic::getFileList(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return (FileList *) it->requestData();
    }
  }
  return NULL;
}

std::string SiteLogic::getRawCommandResult(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      std::string ret = *(std::string *) it->requestData();
      return ret;
    }
  }
  return "";
}

bool SiteLogic::requestStatus(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return it->requestStatus();
    }
  }
  return false;
}

bool SiteLogic::finishRequest(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      bool status = it->requestStatus();
      clearReadyRequest(*it);
      requestsready.erase(it);
      return status;
    }
  }
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    if (connstatetracker[i].hasRequest() &&
        connstatetracker[i].getRequest()->requestId() == requestid)
    {
      connstatetracker[i].finishRequest();
      available++;
      return true;
    }
  }
  global->getEventLog()->log("SiteLogic", "BUG: Couldn't find request to finish.");
  return false;
}

const Pointer<Site> & SiteLogic::getSite() const {
  return site;
}

SiteRace * SiteLogic::getRace(const std::string & race) const {
  for (std::vector<SiteRace *>::const_iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) return *it;
  }
  return NULL;
}

bool SiteLogic::lockDownloadConn(FileList * fl, int * ret, CommandOwner * co, TransferMonitor * tm) {
  return lockTransferConn(fl, ret, tm, co, true);
}

bool SiteLogic::lockUploadConn(FileList * fl, int * ret, CommandOwner * co, TransferMonitor * tm) {
  return lockTransferConn(fl, ret, tm, co, false);
}

bool SiteLogic::lockTransferConn(FileList * fl, int * ret, TransferMonitor * tm, CommandOwner * co, bool isdownload) {
  int lastreadyid = -1;
  bool foundreadythread = false;
  const Path & path = fl->getPath();
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
        !conns[i]->isProcessing())
    {
      foundreadythread = true;
      lastreadyid = i;
      if (conns[i]->getCurrentPath() == path) {
        if (!getSlot(isdownload)) return false;
        *ret = i;
        connstatetracker[i].lockForTransfer(tm, fl, co, isdownload);
        conns[i]->setRawBufferCallback(tm);
        return true;
      }
    }
  }
  if (!foundreadythread) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isHardLocked()) {
        foundreadythread = true;
        if (lastreadyid == -1) {
          lastreadyid = i;
          if (conns[i]->getTargetPath() == path) {
            if (!getSlot(isdownload)) return false;
            *ret = i;
            connstatetracker[i].lockForTransfer(tm, fl, co, isdownload);
            conns[i]->setRawBufferCallback(tm);
            handleConnection(i);
            return true;
          }
        }
      }
    }
  }
  if (foundreadythread) {
    if (!getSlot(isdownload)) return false;
    *ret = lastreadyid;
    connstatetracker[lastreadyid].lockForTransfer(tm, fl, co, isdownload);
    conns[lastreadyid]->setRawBufferCallback(tm);
    handleConnection(lastreadyid);
    return true;
  }
  else return false;
}

void SiteLogic::returnConn(int id, bool istransfer) {
  if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_DOWNLOAD) {
    transferComplete(id, true);
  }
  else if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_UPLOAD) {
    transferComplete(id, false);
  }
  if (istransfer) {
    connstatetracker[id].finishFileTransfer();
  }
  else {
    connstatetracker[id].finishTransfer();
  }
  handleConnection(id);
}

void SiteLogic::setNumConnections(unsigned int num) {
  while (num < conns.size()) {
    bool success = false;
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (!conns[i]->isConnected()) {
        connstatetracker.erase(connstatetracker.begin() + i);
        delete conns[i];
        conns.erase(conns.begin() + i);
        success = true;
        break;
      }
    }
    if (success) {
      continue;
    }
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing()) {
        disconnectConn(i);
        connstatetracker.erase(connstatetracker.begin() + i);
        delete conns[i];
        conns.erase(conns.begin() + i);
        success = true;
        break;
      }
    }
    if (success) {
      continue;
    }
    if (conns.size() > 0) {
      disconnectConn(0);
      connstatetracker.erase(connstatetracker.begin());
      delete conns[0];
      conns.erase(conns.begin());
    }
  }
  while (num > conns.size()) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, conns.size()));
  }
  for (unsigned int i = 0; i < conns.size(); i++) {
    conns[i]->setId(i);
  }
  if (maxslotsup < site->getMaxUp()) {
    slotsup += site->getMaxUp() - maxslotsup;
    maxslotsup = site->getMaxUp();
  }
  if (maxslotsdn < site->getMaxDown()) {
    slotsdn += site->getMaxDown() - maxslotsdn;
    maxslotsdn = site->getMaxDown();
  }
  if (maxslotsup > site->getMaxUp()) {
    slotsup -= (maxslotsup - site->getMaxUp());
    maxslotsup = site->getMaxUp();
  }
  if (maxslotsdn > site->getMaxDown()) {
    slotsdn -= (maxslotsdn - site->getMaxDown());
    maxslotsdn = site->getMaxDown();
  }
  ptrack->updateSlots(site->getMaxDown());
}

bool SiteLogic::downloadSlotAvailable() const {
  return (available > 0 && slotsdn > 0);
}

bool SiteLogic::uploadSlotAvailable() const {
  return (available > 0 && slotsup > 0);
}

int SiteLogic::slotsAvailable() const {
  return available;
}

void SiteLogic::transferComplete(int id, bool isdownload) {
  if (isdownload) {
    slotsdn++;
  }
  else slotsup++;
  available++;
  conns[id]->unsetRawBufferCallback();
}

bool SiteLogic::getSlot(bool isdownload) {
  if (isdownload) {
    if (slotsdn <= 0) {
      return false;
    }
    slotsdn--;
  } else {
    if (slotsup <= 0) {
      return false;
    }
    slotsup--;
  }
  available--;
  return true;
}

void SiteLogic::pushPotential(int score, const std::string & file, const Pointer<SiteLogic> & dst) {
  ptrack->pushPotential(score, file, dst, dst->getSite()->getMaxUp());
}

bool SiteLogic::potentialCheck(int score) {
  int max = ptrack->getMaxAvailablePotential();
  if (score > max * 0.65) {
    return true;
  }
  return false;
}

int SiteLogic::getPotential() {
  return ptrack->getMaxAvailablePotential();
}

void SiteLogic::updateName() {
  for (unsigned int i = 0; i < conns.size(); i++) {
    conns[i]->updateName();
  }
}

int SiteLogic::getCurrLogins() const {
  return loggedin;
}

int SiteLogic::getCurrDown() const {
  return site->getMaxDown() - slotsdn;
}

int SiteLogic::getCurrUp() const {
  return site->getMaxUp() - slotsup;
}

void SiteLogic::connectConn(int id) {
  if (!conns[id]->isConnected()) {
    connstatetracker[id].resetIdleTime();
    conns[id]->login();
  }
}

void SiteLogic::disconnectConn(int id) {
  connstatetracker[id].resetIdleTime();
  cleanupConnection(id);
  if (conns[id]->isConnected()) {
    if (connstatetracker[id].isLoggedIn() && !conns[id]->isProcessing()) {
      conns[id]->doQUIT();
    }
    else {
      conns[id]->disconnect();
    }
  }
  if (connstatetracker[id].isLoggedIn()) {
    loggedin--;
    available--;
  }
  connstatetracker[id].setDisconnected();
}

void SiteLogic::cleanupConnection(int id) {
  while (connstatetracker[id].isListOrTransferLocked()) {
    reportTransferErrorAndFinish(id, 3);
  }
  if (connstatetracker[id].hasRequest()) {
    connstatetracker[id].finishRequest();
    available++;
  }
}

void SiteLogic::finishTransferGracefully(int id) {
  util::assert(connstatetracker[id].hasTransfer() &&
               !connstatetracker[id].isListLocked());
  switch (connstatetracker[id].getTransferType()) {
    case CST_DOWNLOAD:
      connstatetracker[id].getTransferMonitor()->sourceComplete();
      transferComplete(id, true);
      break;
    case CST_UPLOAD:
      connstatetracker[id].getTransferMonitor()->targetComplete();
      transferComplete(id, false);
      break;
  }
  connstatetracker[id].finishTransfer();
}

void SiteLogic::listCompleted(int id, int storeid, FileList * fl, CommandOwner * co) {
  const BinaryData & data = global->getLocalStorage()->getStoreContent(storeid);
  conns[id]->setListData(co, fl);
  conns[id]->parseFileList((char *) &data[0], data.size());
  listRefreshed(id);
  global->getLocalStorage()->purgeStoreContent(storeid);
}

void SiteLogic::issueRawCommand(unsigned int id, const std::string & command) {
  int requestid = requestidcounter++;
  SiteLogicRequest request(requestid, REQ_RAW, conns[id]->getCurrentPath().toString(), command, true);
  request.setConnId(id);
  requests.push_back(request);
  if (!conns[id]->isConnected()) {
    connectConn(id);
    return;
  }
  if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isLocked() && !conns[id]->isProcessing()) {
    handleRequest(id);
  }
}

RawBuffer * SiteLogic::getRawCommandBuffer() const {
  return rawcommandrawbuf;
}

RawBuffer * SiteLogic::getAggregatedRawBuffer() const {
  return aggregatedrawbuf;
}

void SiteLogic::raceGlobalComplete() {
  bool stillactive = false;
  for (unsigned int i = 0; i < races.size(); i++) {
    if (!races[i]->isGlobalDone()) {
      stillactive = true;
      break;
    }
  }
  for (std::list<Pointer<TransferJob> >::const_iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    if (!(*it)->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing() && !connstatetracker[i].isLocked()) {
        disconnectConn(i);
      }
    }
  }
}

void SiteLogic::raceLocalComplete(SiteRace * sr, int uploadslotsleft) {
  sr->complete(true);
  bool stillactive = false;
  for (unsigned int i = 0; i < races.size(); i++) {
    if (!races[i]->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    int downloadslots = site->getMaxDown();
    int stillneededslots = downloadslots < uploadslotsleft ? downloadslots : uploadslotsleft;
    int killnum = site->getMaxLogins() - stillneededslots;
    if (killnum < 0) {
      killnum = 0;
    }
    for (unsigned int i = 0; i < conns.size() && killnum > 0; i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing() && !connstatetracker[i].isLocked()) {
        disconnectConn(i);
        killnum--;
      }
    }
  }
}

const std::vector<FTPConn *> * SiteLogic::getConns() const {
  return &conns;
}

FTPConn * SiteLogic::getConn(int id) const {
  std::vector<FTPConn *>::const_iterator it;
  for (it = conns.begin(); it != conns.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return NULL;
}

std::string SiteLogic::getStatus(int id) const {
  int idletime = connstatetracker[id].getTimePassed()/1000;
  if (!conns[id]->isProcessing() && conns[id]->isConnected() && idletime) {
    return "IDLE " + util::int2Str(idletime) + "s";
  }
  return conns[id]->getStatus();
}


void SiteLogic::preparePassiveTransfer(int id, const std::string & file, bool fxp, bool ssl) {
  connstatetracker[id].setTransfer(file, fxp, ssl);
  initTransfer(id);
}

void SiteLogic::prepareActiveTransfer(int id, const std::string & file, const std::string & host, int port, bool ssl) {
  connstatetracker[id].setTransfer(file, host, port, ssl);
  initTransfer(id);
}

void SiteLogic::preparePassiveList(int id, TransferMonitor * tmb, bool ssl) {
  connstatetracker[id].setList(tmb, ssl);
  initTransfer(id);
}

void SiteLogic::prepareActiveList(int id, TransferMonitor * tmb, const std::string & host, int port, bool ssl) {
  connstatetracker[id].setList(tmb, host, port, ssl);
  initTransfer(id);
}

void SiteLogic::download(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doRETR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(id, true);
      connstatetracker[id].finishTransfer();
      handleConnection(id);
    }
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::upload(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doSTOR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(id, false);
      connstatetracker[id].finishTransfer();
      handleConnection(id);
    }
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::list(int id) {
  conns[id]->doLIST();
}

void SiteLogic::listAll(int id) {
  conns[id]->doLISTa();
}

void SiteLogic::abortTransfer(int id) {
  if (connstatetracker[id].transferInitialized() && !connstatetracker[id].getTransferAborted()) {
    transferComplete(id, connstatetracker[id].getTransferType() == CST_DOWNLOAD);
    connstatetracker[id].abortTransfer();
  }
}

void SiteLogic::getFileListConn(int id) {
  getFileListConn(id, false);
}

void SiteLogic::getFileListConn(int id, bool hiddenfiles) {
  if (site->getListCommand() == SITE_LIST_STAT) {
    if (hiddenfiles) {
      conns[id]->doSTATla();
    }
    else {
      conns[id]->doSTAT();
    }
  }
  else {
    FileList * fl = conns[id]->newFileList();
    global->getTransferManager()->getFileList(global->getSiteLogicManager()->getSiteLogic(this), id, hiddenfiles, fl, NULL);
  }
}

void SiteLogic::getFileListConn(int id, CommandOwner * co, FileList * filelist) {
  util::assert(filelist != NULL);
  if (site->getListCommand() == SITE_LIST_STAT) {
    conns[id]->doSTAT(co, filelist);
  }
  else {
    global->getTransferManager()->getFileList(global->getSiteLogicManager()->getSiteLogic(this), id, false, filelist, co);
  }
}

void SiteLogic::passiveModeCommand(int id) {
  if (connstatetracker[id].getTransferFXP() &&
      connstatetracker[id].getTransferSSL() &&
      site->supportsCPSV()) {
    conns[id]->doCPSV();
  }
  else {
    conns[id]->doPASV();
  }
}

const ConnStateTracker * SiteLogic::getConnStateTracker(int id) const {
  return &connstatetracker[id];
}

void SiteLogic::setRequestReady(unsigned int id, void * data, bool status) {
  const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
  requestsready.push_back(SiteLogicRequestReady(request->requestType(), request->requestId(), data, status));
  if (requestsready.size() > MAXREQUESTREADYQUEUE) {
    clearReadyRequest(requestsready.front());
    requestsready.pop_front();
  }
  const bool care = request->doesAnyoneCare();
  connstatetracker[id].finishRequest();
  available++;
  if (care) {
    global->getUIBase()->backendPush();
  }
}

void SiteLogic::clearReadyRequest(SiteLogicRequestReady & request) {
  void * data = request.requestData();
  if (request.requestStatus()) {
    if (request.getType() == REQ_FILELIST) {
      //delete (FileList *) data;
    }
    else if (request.getType() == REQ_RAW) {
      delete (std::string *) data;
    }
  }
}
