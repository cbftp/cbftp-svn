#include "sitelogic.h"

#include "sitemanager.h"
#include "ftpconn.h"
#include "filelist.h"
#include "siterace.h"
#include "site.h"
#include "globalcontext.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"
#include "sitelogicrequest.h"
#include "sitelogicrequestready.h"
#include "uibase.h"
#include "tickpoke.h"
#include "rawbuffer.h"
#include "eventreceiver.h"
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
#include "types.h"
#include "race.h"

SiteLogic::SiteLogic(std::string sitename) :
  site(global->getSiteManager()->getSite(sitename)),
  rawbuf(new RawBuffer(site->getName())),
  maxslotsup(site->getMaxUp()),
  maxslotsdn(site->getMaxDown()),
  slotsdn(maxslotsdn),
  slotsup(maxslotsup),
  available(0),
  ptrack(new PotentialTracker(slotsdn)),
  loggedin(0),
  requestidcounter(0),
  poke(false)
{
  global->getTickPoke()->startPoke(this, "SiteLogic", 50, 0);

  for (unsigned int i = 0; i < site->getMaxLogins(); i++) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, i));
  }
}

SiteLogic::~SiteLogic() {
  global->getTickPoke()->stopPoke(this, 0);
  delete rawbuf;
  delete ptrack;
  for (unsigned int i = 0; i < conns.size(); i++) {
    delete conns[i];
  }
  for (unsigned int i = 0; i < races.size(); i++) {
    delete races[i];
  }
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

SiteRace * SiteLogic::addRace(Pointer<Race> & enginerace, std::string section, std::string release) {
  SiteRace * race = new SiteRace(enginerace, site->getName(), site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activateAll();
  return race;
}

void SiteLogic::addTransferJob(Pointer<TransferJob> tj) {
  transferjobs.push_back(tj);
  activateOne();
}

void SiteLogic::tick(int message) {
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(50);
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
  CommandOwner * currentco = conns[id]->currentCommandOwner();
  if (currentco != NULL) {
    currentco->fileListUpdated(conns[id]->currentFileList());
  }
  if (connstatetracker[id].getRecursiveLogic()->isActive()) {
    handleRecursiveLogic(id, conns[id]->currentFileList());
    return;
  }
  if (connstatetracker[id].hasRequest()) {
    const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
    FileList * filelist = conns[id]->currentFileList();
    if (request->requestType() == REQ_FILELIST &&
        request->requestData() == filelist->getPath())
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

void SiteLogic::unexpectedResponse(int id) {
  connstatetracker[id].setDisconnected();
}

void SiteLogic::commandSuccess(int id) {
  connstatetracker[id].resetIdleTime();
  int state = conns[id]->getState();
  std::list<SiteLogicRequest>::iterator it;
  switch (state) {
    case STATE_PASS: // PASS, logged in
    case STATE_TYPEI: // performed in chain after PASS if needed
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
    case STATE_CWD:
      if (conns[id]->hasMKDCWDTarget()) {
        if (conns[id]->getCurrentPath() == conns[id]->getMKDCWDTargetSection() +
            "/" + conns[id]->getMKDCWDTargetPath()) {
          conns[id]->finishMKDCWDTarget();
          CommandOwner * currentco = conns[id]->currentCommandOwner();
          if (currentco != NULL && currentco->classType() == COMMANDOWNER_SITERACE) {
            ((SiteRace *)currentco)->addVisitedPath(conns[id]->getCurrentPath());
          }
        }
      }
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_FILELIST &&
            conns[id]->getCurrentPath() == request->requestData())
        {
          getFileListConn(id);
          return;
        }
      }
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      break;
    case STATE_MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        std::string targetcwdsect = conns[id]->getMKDCWDTargetSection();
        std::string targetcwdpath = conns[id]->getMKDCWDTargetPath();
        std::string targetpath = conns[id]->getTargetPath();
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        if (currentco != NULL && currentco->classType() == COMMANDOWNER_SITERACE) {
          ((SiteRace *)currentco)->addVisitedPath(targetpath);
        }
        std::list<std::string> * subdirs = conns[id]->getMKDSubdirs();
        if (targetpath == targetcwdsect + "/" + targetcwdpath) {
          conns[id]->doCWD(targetpath);
          return;
        }
        else if (subdirs->size() > 0) {
          std::string sub = subdirs->front();
          subdirs->pop_front();
          conns[id]->doMKD(targetpath + "/" + sub);
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
      // no action yet, maybe for stats later on
      /*if (connstatetracker[id].isReady()) {
        std::cout << "NOT BUSY!>" << site->getName() << id << " " << connstatetracker[id].getTransferFile()<< connstatetracker[id].isIdle() << std::endl;
        sleep(5);
      }*/
      return;
    case STATE_RETR_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->sourceComplete();
        transferComplete(true);
        connstatetracker[id].finishTransfer();
      }
      else {
        global->getEventLog()->log("SiteLogic", "BUG: returned successfully from RETR without having a transfer. Shouldn't happen!");
      }
      break;
    case STATE_STOR: // STOR started
      // no action yet, maybe for stats later on
      /*if (connstatetracker[id].isReady()) {
        std::cout << "NOT BUSY!>" << site->getName() << id << " " << connstatetracker[id].getTransferFile()<< connstatetracker[id].isIdle() << std::endl;
        sleep(5);
      }*/
      return;
    case STATE_STOR_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->targetComplete();
        transferComplete(false);
        connstatetracker[id].finishTransfer();
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
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_DEL) {
          setRequestReady(id, NULL, true);
        }
        else if (request->requestType() == REQ_DEL_RECURSIVE) {
          if (connstatetracker[id].getRecursiveLogic()->isActive()) {
            handleRecursiveLogic(id);
            return;
          }
          else {
            setRequestReady(id, NULL, true);
            break;
          }
        }
      }
      break;
    case STATE_NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_NUKE) {
          setRequestReady(id, NULL, true);
        }
      }
      break;
    case STATE_LIST: // LIST started, no action here
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
  handleConnection(id, false);
}

void SiteLogic::commandFail(int id) {
  connstatetracker[id].resetIdleTime();
  int state = conns[id]->getState();
  std::string targetcwdsect;
  std::string targetcwdpath;
  std::list<std::string> * subdirs;
  std::string file;
  std::list<SiteLogicRequest>::iterator it;
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
        handleConnection(id, false);
        return;
      }
      break;
    case STATE_CWD:
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        connstatetracker[id].getRecursiveLogic()->failedCwd();
        handleRecursiveLogic(id);
        return;
      }
      if (conns[id]->hasMKDCWDTarget()) {
        if (!site->getAllowUpload() ||
            site->isAffiliated(((SiteRace *)conns[id]->currentCommandOwner())->getGroup())) {
          conns[id]->finishMKDCWDTarget();
          handleFail(id);
          return;
        }
        conns[id]->doMKD(conns[id]->getTargetPath());
        return;
      }
      if (connstatetracker[id].transferInitialized()) {
        handleTransferFail(id, TM_ERR_OTHER);
        return;
      }
      if (connstatetracker[id].hasTransfer()) {
        handleFail(id);
        return;
      }
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_FILELIST) {
          setRequestReady(id, NULL, false);
          handleConnection(id, false);
          return;
        }
        break;
      }
      {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        if (currentco != NULL && currentco->classType() == COMMANDOWNER_SITERACE &&
            ((SiteRace *)currentco)->pathVisited(conns[id]->getTargetPath())) {
          handleFail(id);
          return;
        }
      }
      break;
    case STATE_MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        targetcwdsect = conns[id]->getMKDCWDTargetSection();
        targetcwdpath = conns[id]->getMKDCWDTargetPath();
        subdirs = conns[id]->getMKDSubdirs();
        if (conns[id]->getTargetPath() == targetcwdsect + "/" + targetcwdpath) {
          if (subdirs->size() > 0) {
            std::string sub = subdirs->front();
            subdirs->pop_front();
            std::string newattempt = targetcwdsect + "/" + sub;
            CommandOwner * currentco = conns[id]->currentCommandOwner();
            if (currentco != NULL && currentco->classType() == COMMANDOWNER_SITERACE) {
              if (((SiteRace *)currentco)->pathVisited(newattempt)) {
                handleFail(id);
                return;
              }
              ((SiteRace *)currentco)->addVisitedPath(newattempt);
            }
            conns[id]->doMKD(newattempt);
            return;
          }
          else {
            handleFail(id);
            return;
          }
        }
        else {
          // cwdmkd failed.
        }
      }
      else {
        CommandOwner * currentco = conns[id]->currentCommandOwner();
        if (currentco != NULL && currentco->classType() == COMMANDOWNER_TRANSFERJOB) {
          handleConnection(id, false);
          return;
        }
      }
      break;
    case STATE_PRET_RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_PRET);
      return;
    case STATE_PRET_STOR:
      handleTransferFail(id, CST_UPLOAD, TM_ERR_PRET);
      return;
    case STATE_RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR);
      return;
    case STATE_RETR_COMPLETE:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR_COMPLETE);
      return;
    case STATE_STOR:
      handleTransferFail(id, CST_UPLOAD, TM_ERR_RETRSTOR);
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
      handleConnection(id, false);
      return;
    case STATE_DELE:
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      if (connstatetracker[id].hasRequest()) {
        const Pointer<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_DEL_RECURSIVE ||
            request->requestType() == REQ_DEL)
        {
          setRequestReady(id, NULL, false);
        }
      }
      handleConnection(id, false);
      return;
    case STATE_NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->requestType() == REQ_NUKE) {
          setRequestReady(id, NULL, false);
        }
      }
      handleConnection(id, false);
      return;
    case STATE_LIST:
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR);
      return;
    case STATE_PRET_LIST:
      handleTransferFail(id, CST_LIST, TM_ERR_PRET);
      return;
    case STATE_LIST_COMPLETE:
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR_COMPLETE);
      return;
  }
  // default handling: reconnect
  disconnected(id);
  conns[id]->reconnect();
}

void SiteLogic::handleFail(int id) {
  if (connstatetracker[id].transferInitialized()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return;
  }
  if (connstatetracker[id].isLocked()) {
    handleConnection(id, false);
    return;
  }
  connstatetracker[id].delayedCommand("handle", SLEEPDELAY * 6);
}

void SiteLogic::handleTransferFail(int id, int err) {
  if (connstatetracker[id].transferInitialized()) {
    handleTransferFail(id, connstatetracker[id].getTransferType(), err);
  }
  else {
    global->getEventLog()->log("SiteLogic", "BUG: Returned failed transfer (code " +
        util::int2Str(err) + ") without having a transfer, shouldn't happen!");
  }
}

void SiteLogic::handleTransferFail(int id, int type, int err) {
  if (connstatetracker[id].transferInitialized()) {
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
  if (err == 1) {
    conns[id]->abortTransferPASV();
  }
  else if (err == 3 && !connstatetracker[id].isLocked()) {
    connstatetracker[id].delayedCommand("handle", SLEEPDELAY * 6);
  }
  else {
    handleConnection(id, false);
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
        transferComplete(true);
        break;
      case CST_LIST:
        connstatetracker[id].getTransferMonitor()->sourceError((TransferError)err);
        break;
      case CST_UPLOAD:
        connstatetracker[id].getTransferMonitor()->targetError((TransferError)err);
        transferComplete(false);
        break;
    }
  }
  connstatetracker[id].finishTransfer();
}

void SiteLogic::gotPath(int id, std::string path) {
  connstatetracker[id].resetIdleTime();
}

void SiteLogic::rawCommandResultRetrieved(int id, std::string result) {
  connstatetracker[id].resetIdleTime();
  rawbuf->write(result);
  if (connstatetracker[id].hasRequest()) {
    if (connstatetracker[id].getRequest()->requestType() == REQ_RAW) {
      setRequestReady(id, NULL, true);
    }
  }
  handleConnection(id, true);
}

void SiteLogic::gotPassiveAddress(int id, std::string result) {
  connstatetracker[id].resetIdleTime();
  int count = 0;
  for (unsigned int i = 0; i < result.length(); i++) {
    if (result[i] == ',') count++;
  }
  if (count == 2 && result.substr(0, 2) == "1,") {
    std::string addr = conns[id]->getConnectedAddress();
    for (unsigned int i = 0; i < addr.length(); i++) {
      if (addr[i] == '.') addr[i] = ',';
    }
    result = addr + "," + result.substr(2);
  }
  if (connstatetracker[id].transferInitialized()) {
    connstatetracker[id].getTransferMonitor()->passiveReady(result);
  }
  else {
    handleConnection(id, false);
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
          !conns[i]->isProcessing()) {
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

void SiteLogic::haveConnected(unsigned int connected) {
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
    return;
  }
  if (requests.size() > 0) {
    if (handleRequest(id)) {
      return;
    }
  }
  Pointer<TransferJob> targetjob;
  for (std::list<Pointer<TransferJob> >::iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    Pointer<TransferJob> tj = *it;
    if (!tj->isDone()) {
      if (tj->wantsList(this)) {
        FileList * fl = tj->getListTarget(this);
        std::string path = fl->getPath();
        connstatetracker[id].use();
        if (path != conns[id]->getCurrentPath()) {
          conns[id]->doCWD(path);
          return;
        }
        getFileListConn(id, tj.get(), fl);
        return;
      }
      if (tj->wantsMakeDir(this)) {
        connstatetracker[id].use();
        conns[id]->setCurrentCommandOwner(tj.get());
        conns[id]->doMKD(tj->getWantedMakeDir());
        return;
      }
      if (!tj->isInitialized()) {
        targetjob = tj;
        continue;
      }
      int type = tj->getType();
      if (!targetjob &&
          (((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_DOWNLOAD_FILE ||
          ((type == TRANSFERJOB_FXP || type == TRANSFERJOB_FXP_FILE) && tj->getSrc() == this)) &&
          getCurrDown() < tj->maxSlots()) ||
          ((type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_UPLOAD_FILE ||
          ((type == TRANSFERJOB_FXP || type == TRANSFERJOB_FXP_FILE) && tj->getDst() == this)) &&
          getCurrUp() < tj->maxSlots()))) {
        targetjob = tj;
        continue;
      }
    }
  }
  if (!!targetjob) {
    connstatetracker[id].delayedCommand("handle", SLEEPDELAY);
    if (global->getEngine()->transferJobActionRequest(targetjob)) {
      return;
    }
    else {
      connstatetracker[id].use();
    }
  }
  SiteRace * race = NULL;
  bool refresh = false;
  SiteRace * lastchecked = connstatetracker[id].lastChecked();
  if (lastchecked && !lastchecked->isDone() && connstatetracker[id].checkCount() < MAXCHECKSROW) {
    race = lastchecked;
    refresh = true;
    connstatetracker[id].check(race);
  }
  else {
    for (unsigned int i = 0; i < races.size(); i++) {
      if (!races[i]->isDone() && !wasRecentlyListed(races[i])) {
        race = races[i];
        break;
      }
    }
    if (race == NULL) {
      for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
        if (!(*it)->isDone()) {
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
      if (!races[i]->isGlobalDone()) {
        race = races[i];
        break;
      }
    }
  }
  if (race != NULL) {
    std::string currentpath = conns[id]->getCurrentPath();
    size_t slashpos = currentpath.rfind("/");
    std::string racepath = race->getPath();
    bool goodpath = (currentpath.length() == racepath.length() && currentpath.compare(racepath) == 0) || // same path
        (slashpos == racepath.length() && currentpath.substr(0, slashpos).compare(racepath) == 0); // same path, currently in subdir
    if (goodpath && !refresh) {
      return;
    }
    if (backfromrefresh) {
      connstatetracker[id].delayedCommand("refreshchangepath", SLEEPDELAY, (void *) race);
      return;
    }
    if (goodpath) {
      std::string subpath = slashpos == racepath.length() ? currentpath.substr(slashpos + 1) : "";
      FileList * fl = race->getFileListForPath(subpath);
      if (fl == NULL) {
        race->addSubDirectory(subpath);
        fl = race->getFileListForPath(subpath);
      }
      connstatetracker[id].use();
      getFileListConn(id, race, fl);
      return;
    }
    else {
      connstatetracker[id].use();
      refreshChangePath(id, race, refresh);
    }
  }
  else {
    if (site->getMaxIdleTime()) {
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
  std::string targetpath;
  std::string actiontarget;
  switch (it->requestType()) {
    case REQ_FILELIST: // filelist
      targetpath = it->requestData();
      if (conns[id]->getCurrentPath() == targetpath) {
        getFileListConn(id);
      }
      else {
        conns[id]->doCWD(targetpath);
      }
      break;
    case REQ_RAW: // raw command
      rawbuf->writeLine(it->requestData());
      conns[id]->doRaw(it->requestData());
      break;
    case REQ_WIPE_RECURSIVE: // recursive wipe
      conns[id]->doWipe(it->requestData(), true);
      break;
    case REQ_WIPE: // wipe
      conns[id]->doWipe(it->requestData(), false);
      break;
    case REQ_DEL_RECURSIVE: // recursive delete
      targetpath = it->requestData();
      connstatetracker[id].getRecursiveLogic()->initialize(RCL_DELETE, conns[id]->getCurrentPath(), targetpath);
      handleRecursiveLogic(id);
      break;
    case REQ_DEL: // delete
      conns[id]->doDELE(it->requestData());
      break;
    case REQ_NUKE: // nuke
      conns[id]->doNuke(it->requestData(), it->requestData3(), it->requestData2());
      break;
  }
  connstatetracker[id].setRequest(*it);
  available--;
  requests.erase(it);
  return true;
}

void SiteLogic::handleRecursiveLogic(int id) {
  handleRecursiveLogic(id, NULL);
}

void SiteLogic::handleRecursiveLogic(int id, FileList * fl) {
  std::string actiontarget;
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
  std::string currentpath = conns[id]->getCurrentPath();
  std::string subpath = race->getRelevantSubPath();
  std::string appendsubpath = subpath;
  conns[id]->setCurrentCommandOwner(race);
  if (appendsubpath.length() > 0) {
    appendsubpath = "/" + subpath;
  }
  std::string targetpath = race->getPath() + appendsubpath;
  if (targetpath != currentpath) {
    if (!race->pathVisited(targetpath)) {
      conns[id]->setMKDCWDTarget(race->getSection(), race->getRelease() + appendsubpath);
    }
    conns[id]->doCWD(targetpath);
  }
  else {
    if (refresh) {
      getFileListConn(id, race, race->getFileListForFullPath(currentpath));
    }
  }
}

void SiteLogic::initTransfer(int id) {
  if (!connstatetracker[id].transferInitialized()) {
    connstatetracker[id].initializeTransfer();
  }
  int transfertype = connstatetracker[id].getTransferType();
  bool transferssl = connstatetracker[id].getTransferSSL();
  if (transfertype != CST_LIST) {
    std::string transferpath = connstatetracker[id].getTransferPath();
    if (conns[id]->getCurrentPath() != transferpath) {
      conns[id]->doCWD(transferpath);
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
      conns[id]->doPORT(connstatetracker[id].getTransferAddr());
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

int SiteLogic::requestFileList(std::string path) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_FILELIST, path, true));
  activateOne();
  return requestid;
}

int SiteLogic::requestRawCommand(std::string command, bool care) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_RAW, command, care));
  activateOne();
  return requestid;
}

int SiteLogic::requestWipe(std::string path, bool recursive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE_RECURSIVE, path, true));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE, path, true));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestDelete(std::string path, bool recursive, bool interactive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.push_back(SiteLogicRequest(requestid, REQ_DEL_RECURSIVE, path, interactive));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_DEL, path, interactive));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestNuke(std::string path, int multiplier, std::string reason) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_NUKE, path, reason, multiplier, true));
  activateOne();
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
      delete (std::string *) it->requestData();
      return ret;
    }
  }
  return "";
}

bool SiteLogic::finishRequest(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      bool status = it->requestStatus();
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

Site * SiteLogic::getSite() const {
  return site;
}

SiteRace * SiteLogic::getRace(std::string race) const {
  for (std::vector<SiteRace *>::const_iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) return *it;
  }
  return NULL;
}

bool SiteLogic::lockDownloadConn(std::string path, int * ret, TransferMonitor * tm) {
  return lockTransferConn(path, ret, tm, true);
}

bool SiteLogic::lockUploadConn(std::string path, int * ret, TransferMonitor * tm) {
  return lockTransferConn(path, ret, tm, false);
}

bool SiteLogic::lockTransferConn(std::string path, int * ret, TransferMonitor * tm, bool isdownload) {
  int lastreadyid = -1;
  bool foundreadythread = false;
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
        !conns[i]->isProcessing()) {
      foundreadythread = true;
      lastreadyid = i;
      if (conns[i]->getTargetPath().compare(path) == 0) {
        if (!getSlot(isdownload)) return false;
        *ret = i;
        connstatetracker[i].lockForTransfer(tm, isdownload);
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
          if (conns[i]->getTargetPath().compare(path) == 0) {
            if (!getSlot(isdownload)) return false;
            *ret = i;
            connstatetracker[i].lockForTransfer(tm, isdownload);
            return true;
          }
        }
      }
    }
  }
  if (foundreadythread) {
    if (!getSlot(isdownload)) return false;
    if (!conns[lastreadyid]->isProcessing() && !connstatetracker[lastreadyid].isListLocked()) {
      conns[lastreadyid]->doCWD(path);
    }
    *ret = lastreadyid;
    connstatetracker[lastreadyid].lockForTransfer(tm, isdownload);
    return true;
  }
  else return false;
}

void SiteLogic::returnConn(int id) {
  util::assert(!connstatetracker[id].isListLocked());
  if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_DOWNLOAD) {
    transferComplete(true);
  }
  else if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_UPLOAD) {
    transferComplete(false);
  }
  connstatetracker[id].finishTransfer();
  handleConnection(id, false);
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

void SiteLogic::transferComplete(bool isdownload) {
  if (isdownload) {
    slotsdn++;
  }
  else slotsup++;
  available++;
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

void SiteLogic::pushPotential(int score, std::string file, SiteLogic * dst) {
  int threads = getSite()->getMaxDown();
  ptrack->getFront()->update(dst, threads, dst->getSite()->getMaxDown(), score, file);
}

bool SiteLogic::potentialCheck(int score) {
  int max = ptrack->getMaxAvailablePotential();
  if (score > max/2) {
    return true;
  }
  return false;
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
      transferComplete(true);
      break;
    case CST_UPLOAD:
      connstatetracker[id].getTransferMonitor()->targetComplete();
      transferComplete(false);
      break;
  }
  connstatetracker[id].finishTransfer();
}

void SiteLogic::listCompleted(int id, int storeid) {
  const binary_data & data = global->getLocalStorage()->getStoreContent(storeid);
  conns[id]->parseFileList((char *) &data[0], data.size());
  listRefreshed(id);
  global->getLocalStorage()->purgeStoreContent(storeid);
}

void SiteLogic::issueRawCommand(unsigned int id, std::string command) {
  int requestid = requestidcounter++;
  SiteLogicRequest request(requestid, REQ_RAW, command, true);
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
  return rawbuf;
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


void SiteLogic::preparePassiveTransfer(int id, std::string path, std::string file, bool fxp, bool ssl) {
  connstatetracker[id].setTransfer(path, file, fxp, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::preparePassiveList(int id, TransferMonitor * tmb, bool ssl) {
  connstatetracker[id].setList(tmb, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::download(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doRETR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(true);
      connstatetracker[id].finishTransfer();
      handleConnection(id, false);
    }
  }
  else {
    handleConnection(id, false);
  }
}

void SiteLogic::upload(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doSTOR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(false);
      connstatetracker[id].finishTransfer();
      handleConnection(id, false);
    }
  }
  else {
    handleConnection(id, false);
  }
}

void SiteLogic::list(int id) {
  conns[id]->doLIST();
}

void SiteLogic::listAll(int id) {
  conns[id]->doLISTa();
}

void SiteLogic::prepareActiveTransfer(int id, std::string path, std::string file, std::string addr, bool ssl) {
  connstatetracker[id].setTransfer(path, file, addr, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::abortTransfer(int id) {
  if (connstatetracker[id].transferInitialized() && !connstatetracker[id].getTransferAborted()) {
    transferComplete(connstatetracker[id].getTransferType() == CST_DOWNLOAD);
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
    conns[id]->prepareLIST();
    global->getTransferManager()->getFileList(this, id, hiddenfiles);
  }
}

void SiteLogic::getFileListConn(int id, CommandOwner * co, FileList * filelist) {
  util::assert(filelist != NULL);
  if (site->getListCommand() == SITE_LIST_STAT) {
    conns[id]->doSTAT(co, filelist);
  }
  else {
    conns[id]->prepareLIST(co, filelist);
    global->getTransferManager()->getFileList(this, id, false);
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
  requestsready.push_back(SiteLogicRequestReady(request->requestId(), data, status));
  if (requestsready.size() > MAXREQUESTREADYQUEUE) {
    requestsready.pop_front();
  }
  const bool care = request->doesAnyoneCare();
  connstatetracker[id].finishRequest();
  available++;
  if (care) {
    global->getUIBase()->backendPush();
  }
}
