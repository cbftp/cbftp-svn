#include "sitelogic.h"

#include <cassert>

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
#include "race.h"
#include "sitetransferjob.h"
#include "transferstatus.h"
#include "filelistdata.h"

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEP_DELAY 150

//maximum number of dir refreshes in a row in the same race
#define MAX_CHECKS_ROW 5

// maximum number of ready requests available to be checked out, unless below
#define MAX_REQUEST_READY_QUEUE_SIZE 10

// minimum time that requests are guaranteed to live in the ready queue, in ms
#define MAX_REQUEST_READY_QUEUE_IDLE_TIME 10000

// ticker heartbeat, in ms
#define TICK_INTERVAL 50

// time that a filelist is considered newly updated and not in need of refresh,
// in ms
#define FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH 2000

// time that a filelist is considered newly updated and not in need of refresh
// when xdupe is enabled, in ms
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
  REQ_IDLE,
  REQ_MKDIR
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
  slotsdn(0),
  slotsup(0),
  available(0),
  ptrack(new PotentialTracker(site->getMaxDown())),
  loggedin(0),
  requestidcounter(0),
  poke(false),
  currtime(0),
  timesincelastrequestready(0)
{
  global->getTickPoke()->startPoke(this, "SiteLogic", TICK_INTERVAL, 0);

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

std::shared_ptr<SiteRace> SiteLogic::addRace(std::shared_ptr<Race> & enginerace, const std::string & section, const std::string & release) {
  std::shared_ptr<SiteRace> race = std::make_shared<SiteRace>(enginerace, site->getName(), site->getSectionPath(section),
                                                              release, site->getUser(), site->getSkipList(), site->isAffiliated(enginerace->getGroup()));
  currentraces.push_back(race);
  activateAll();
  return race;
}

void SiteLogic::resetRace(const std::shared_ptr<SiteRace> & race) {
  bool current = false;
  for (const std::shared_ptr<SiteRace> & currentrace : currentraces) {
    if (currentrace == race) {
      current = true;
      break;
    }
  }
  if (!current) {
    currentraces.push_back(race);
  }
  activateAll();
}

void SiteLogic::addTransferJob(std::shared_ptr<SiteTransferJob> & tj) {
  transferjobs.push_back(tj);
  activateOne();
}

void SiteLogic::tick(int message) {
  currtime += TICK_INTERVAL;
  timesincelastrequestready += TICK_INTERVAL;
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(TICK_INTERVAL);
    if (connstatetracker[i].getCommand().isReleased()) {
      assert(!connstatetracker[i].isLocked() && !conns[i]->isProcessing());
      DelayedCommand eventcommand = connstatetracker[i].getCommand();
      connstatetracker[i].getCommand().reset();
      connstatetracker[i].use();
      std::string event = eventcommand.getCommand();
      if (event == "refreshchangepath") {
        std::shared_ptr<SiteRace> race = std::static_pointer_cast<SiteRace>(eventcommand.getCommandOwner());
        if (!race) {
          handleConnection(i, false);
        }
        else {
          refreshChangePath(i, race, true);
        }
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
  if (currtime % 3600000 == 0) {
    site->tickHour();
  }
}

void SiteLogic::connectFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::userDenied(int id) {
  disconnectConn(id);
}

void SiteLogic::userDeniedSiteFull(int id) {
  connstatetracker[id].delayedCommand("reconnect", SLEEP_DELAY, true);
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
  std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
  if (!!currentco) {
    currentco->fileListUpdated(this, conns[id]->currentFileList());
  }
  if (connstatetracker[id].getRecursiveLogic()->isActive()) {
    handleRecursiveLogic(id, conns[id]->currentFileList());
    return;
  }
  if (connstatetracker[id].hasRequest()) {
    const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
    FileList * filelist = conns[id]->currentFileList();
    FileListData * filelistdata = new FileListData(filelist, conns[id]->getCwdRawBuffer());
    if (request->requestType() == REQ_FILELIST &&
        request->requestData() == filelist->getPath().toString())
    {
      setRequestReady(id, filelistdata, true);
    }
  }
  if (!!currentco) {
    global->getEngine()->jobFileListRefreshed(this, currentco, fl);
  }
  handleConnection(id, true);
}

bool SiteLogic::setPathExists(int id, int exists, bool refreshtime) {
  FileList * fl = conns[id]->currentFileList();
  if (!fl) {
    return false;
  }
  bool statechanged = false;
  FileListState state = fl->getState();
  if (state == FileListState::UNKNOWN) {
    if (exists == EXISTS_YES) {
      fl->setExists();
    }
    else if (exists == EXISTS_FAILED) {
      fl->setFailed();
    }
    else if (exists == EXISTS_NO) {
      fl->setNonExistent();
    }
    statechanged = true;
  }
  else if (state == FileListState::NONEXISTENT) {
    if (exists == EXISTS_YES) {
      fl->setExists();
      statechanged = true;
    }
    else if (exists == EXISTS_FAILED) {
      fl->setFailed();
      statechanged = true;
    }
  }
  else if (state == FileListState::FAILED) {
    if (exists == EXISTS_YES) {
      fl->setExists();
      statechanged = true;
    }
  }
  else if (state == FileListState::LISTED || state == FileListState::EXISTS) {
    if (exists == EXISTS_NO) {
      if (fl->addListFailure()) {
        statechanged = true;
      }
    }
  }
  if (statechanged && refreshtime) {
    fl->setRefreshedTime(currtime);
  }
  std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
  if (exists != EXISTS_YES) {
    if (fl->bumpUpdateState(UpdateState::REFRESHED)) {
      statechanged = true;
    }
  }
  if (currentco && statechanged) {
    currentco->fileListUpdated(this, fl);
  }
  return statechanged;
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

bool SiteLogic::makeTargetDirectory(int id, bool includinglast, const std::shared_ptr<CommandOwner> & co) {
  Path trypath = conns[id]->getMKDCWDTargetSection();
  std::list<std::string> subdirs = conns[id]->getMKDSubdirs();
  while (co && subdirs.size() > (includinglast ? 0 : 1)) {
    trypath = trypath / subdirs.front();
    subdirs.pop_front();
    FileList * fl = co->getFileListForFullPath(this, trypath);
    if (fl) {
      if (fl->getState() == FileListState::UNKNOWN || fl->getState() == FileListState::NONEXISTENT) {
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

void SiteLogic::commandSuccess(int id, int state) {
  connstatetracker[id].resetIdleTime();
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
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_FILELIST) {
          getFileListConn(id);
          return;
        }
        else if (request->requestType() == REQ_RAW) {
          rawcommandrawbuf->writeLine(request->requestData2());
          conns[id]->doRaw(request->requestData2());
          return;
        }
        else if (request->requestType() == REQ_IDLE) {
          setRequestReady(id, NULL, true);
          handleConnection(id);
          if (!conns[id]->isProcessing()) {
            connstatetracker[id].delayedCommand("quit", request->requestData3() * 1000);
          }
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
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_MKDIR) {
          setRequestReady(id, NULL, true);
        }
      }
      if (conns[id]->hasMKDCWDTarget()) {
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        setPathExists(id, EXISTS_YES, true);
        if (targetpath == targetcwdsect / targetcwdpath) {
          conns[id]->finishMKDCWDTarget();
          FileList * fl = conns[id]->currentFileList();
          if (fl) {
            conns[id]->doCWD(fl, currentco);
          }
          else {
            conns[id]->doCWD(targetpath, currentco);
          }
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
        assert(connstatetracker[id].getTransferPassive());
        passiveModeCommand(id);
        return;
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
      assert(connstatetracker[id].transferInitialized());
      connstatetracker[id].getTransferMonitor()->sourceComplete();
      transferComplete(id, true);
      connstatetracker[id].finishTransfer();
      global->getEngine()->raceActionRequest();
      if (conns[id]->isProcessing() || connstatetracker[id].isTransferLocked()) {
        return;
      }
      break;
    case STATE_STOR: // STOR started
      if (connstatetracker[id].transferInitialized()) {
        if (!connstatetracker[id].getTransferPassive()) {
          connstatetracker[id].getTransferMonitor()->activeStarted();
        }
      }
      return;
    case STATE_STOR_COMPLETE: {
      assert(connstatetracker[id].transferInitialized());
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
      break;
    }
    case STATE_ABOR:
      break;
    case STATE_PASV_ABORT:
      break;
    case STATE_WIPE:
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
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
    case STATE_QUIT:
      connstatetracker[id].setDisconnected();
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
      FileList * currentfl = conns[id]->currentFileList();
      std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
      if (conns[id]->hasMKDCWDTarget()) {
        if (site->getAllowUpload() == SITE_ALLOW_TRANSFER_NO ||
            (!!currentco && currentco->classType() == COMMANDOWNER_SITERACE &&
             site->isAffiliated(std::static_pointer_cast<SiteRace>(currentco)->getGroup())))
        {
          conns[id]->finishMKDCWDTarget();
        }
        else {
          if (currentfl) {
            conns[id]->doMKD(currentfl, currentco);
          }
          else {
            conns[id]->doMKD(conns[id]->getTargetPath(), currentco);
          }
          return;
        }
      }
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->requestType() == REQ_FILELIST ||
            request->requestType() == REQ_RAW ||
            request->requestType() == REQ_IDLE)
        {
          setRequestReady(id, NULL, false);
          handleConnection(id);
          if (request->requestType() == REQ_IDLE && !conns[id]->isProcessing()) {
            connstatetracker[id].delayedCommand("quit", request->requestData3() * 1000);
          }
          return;
        }
      }
      if (filelistupdated) {
        if (currentco && currentfl) {
          global->getEngine()->jobFileListRefreshed(this, currentco, currentfl);
        }
        handleConnection(id);
        return;
      }
      if (connstatetracker[id].hasTransfer() && !connstatetracker[id].transferInitialized()) {
        handleConnection(id);
        return;
      }
      handleFail(id);
      return;
    }
    case STATE_MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        if (targetpath == targetcwdsect / targetcwdpath && makeTargetDirectory(id, false, currentco)) {
          return;
        }
        conns[id]->finishMKDCWDTarget();
        setPathExists(id, EXISTS_FAILED, true);
        FileList * fl = conns[id]->currentFileList();
        if (fl) {
          conns[id]->doCWD(fl, currentco);
        }
        else {
          conns[id]->doCWD(targetpath, currentco);
        }
        return;
      }
      else {
        if (connstatetracker[id].hasRequest()) {
          const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
          if (request->requestType() == REQ_MKDIR) {
            setRequestReady(id, NULL, false);
            handleConnection(id);
            return;
          }
        }
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        if (!!currentco && currentco->classType() == COMMANDOWNER_TRANSFERJOB) {
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
        const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
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
  connstatetracker[id].delayedCommand("handle", SLEEP_DELAY);
}

void SiteLogic::handleTransferFail(int id, int err) {
  assert(connstatetracker[id].isListOrTransferLocked());
  handleTransferFail(id, connstatetracker[id].getTransferType(), err);
}

void SiteLogic::handleTransferFail(int id, int type, int err) {
  assert(connstatetracker[id].isListOrTransferLocked());
  reportTransferErrorAndFinish(id, type, err);
  if (connstatetracker[id].hasTransfer() && connstatetracker[id].transferInitialized() &&
      connstatetracker[id].getTransferPassive() &&
      (err == TM_ERR_RETRSTOR || err == TM_ERR_DUPE))
  {
    conns[id]->abortTransferPASV();
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::reportTransferErrorAndFinish(int id, int err) {
  reportTransferErrorAndFinish(id, connstatetracker[id].getTransferType(), err);
}

void SiteLogic::reportTransferErrorAndFinish(int id, int type, int err) {
  std::shared_ptr<CommandOwner> co = connstatetracker[id].getCommandOwner();
  FileList * fl = connstatetracker[id].getTransferFileList();
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
  if (type == CST_UPLOAD && err == TM_ERR_DUPE && site->useXDUPE() && co && fl) {
    const std::list<std::string> & xdupelist = conns[id]->getXDUPEList();
    for (std::list<std::string>::const_iterator it = xdupelist.begin(); it != xdupelist.end(); it++) {
      fl->touchFile(*it, "XDUPE");
    }
    global->getEngine()->jobFileListRefreshed(this, co, fl);
  }
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
  if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isQuitting()) {
    loggedin--;
    available--;
  }
  connstatetracker[id].setDisconnected();
}

void SiteLogic::activateOne() {
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
  std::shared_ptr<CommandOwner> co = connstatetracker[id].getCommandOwner();
  if (conns[id]->getCurrentPath() != transferpath) {
    if (connstatetracker[id].getTransferType() == CST_UPLOAD &&
        fl->getState() == FileListState::NONEXISTENT)
    {
      std::pair<Path, Path> pathparts;
      if (co && co->classType() == COMMANDOWNER_TRANSFERJOB) {
        Path sectionpath = std::static_pointer_cast<SiteTransferJob>(co)->getPath();
        pathparts = std::pair<Path, Path>(sectionpath, transferpath - sectionpath);
      }
      else {
        pathparts = site->splitPathInSectionAndSubpath(transferpath);
      }
      conns[id]->setMKDCWDTarget(pathparts.first, pathparts.second);
      return makeTargetDirectory(id, true, co);
    }
    conns[id]->doCWD(fl, co);
    return true;
  }
  return false;
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
  std::list<std::shared_ptr<SiteTransferJob> > targetjobs;
  for (std::list<std::shared_ptr<SiteTransferJob> >::iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    std::shared_ptr<SiteTransferJob> & tj = *it;
    if (!tj->getTransferJob()->isDone()) {
      if (tj->wantsList()) {
        FileList * fl = tj->getListTarget();
        const Path & path = fl->getPath();
        connstatetracker[id].use();
        if (path != conns[id]->getCurrentPath()) {
          conns[id]->doCWD(fl, tj);
          return;
        }
        getFileListConn(id, tj, fl);
        return;
      }
      int type = tj->getTransferJob()->getType();
      if (((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) &&
            tj->getTransferJob()->getSrc()->getCurrDown() < tj->getTransferJob()->maxSlots()) ||
          (type == TRANSFERJOB_UPLOAD && getCurrUp() < tj->getTransferJob()->maxSlots()))
      {
        targetjobs.push_back(tj);
        continue;
      }
    }
  }
  if (targetjobs.size()) {
    connstatetracker[id].delayedCommand("handle", SLEEP_DELAY);
  }
  for (std::list<std::shared_ptr<SiteTransferJob> >::iterator it = targetjobs.begin(); it != targetjobs.end(); it++) {
    if (global->getEngine()->transferJobActionRequest(*it)) {
      return;
    }
  }
  if (targetjobs.size()) {
    connstatetracker[id].use();
  }
  std::shared_ptr<SiteRace> race;
  bool refresh = false;
  std::shared_ptr<SiteRace> lastchecked = connstatetracker[id].lastChecked();
  if (lastchecked && !lastchecked->isDone() && lastchecked->anyFileListNotNonExistent() &&
      connstatetracker[id].checkCount() < MAX_CHECKS_ROW)
  {
    race = lastchecked;
    refresh = true;
    connstatetracker[id].check(race);
  }
  else {
    for (unsigned int i = 0; i < currentraces.size(); i++) {
      if (!currentraces[i]->isDone() && !wasRecentlyListed(currentraces[i])) {
        race = currentraces[i];
        break;
      }
    }
    if (!race) {
      for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
        if (!(*it)->isDone()) {
          race = *it;
          break;
        }
      }
    }
    if (!!race) {
      refresh = true;
      connstatetracker[id].check(race);
      addRecentList(race);
    }
  }
  if (!race) {
    for (unsigned int i = 0; i < currentraces.size(); i++) {
      if (currentraces[i]->anyFileListNotNonExistent()) {
        race = currentraces[i];
        break;
      }
    }
  }
  if (!race && !currentraces.empty()) {
    race = currentraces.front();
  }
  if (!!race) {
    const Path & currentpath = conns[id]->getCurrentPath();
    const Path & racepath = race->getPath();
    bool goodpath = currentpath == racepath || // same path
                    currentpath.contains(racepath); // same path, currently in subdir
    if (!goodpath || refresh) {
      if (backfromrefresh) {
        connstatetracker[id].delayedCommand("refreshchangepath", SLEEP_DELAY, false, race);
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
        return;
      }
    }
  }
  if (site->getMaxIdleTime() && !connstatetracker[id].getCommand().isActive()) {
    connstatetracker[id].delayedCommand("quit", site->getMaxIdleTime() * 1000);
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
    case REQ_IDLE: { // idle
      Path targetpath = request.requestData();
      if (conns[id]->getCurrentPath() != targetpath) {
        conns[id]->doCWD(targetpath);
      }
      else {
        setRequestReady(id, NULL, true);
        handleConnection(id);
        if (!conns[id]->isProcessing()) {
          connstatetracker[id].delayedCommand("quit", request.requestData3() * 1000);
        }
      }
      break;
    }
    case REQ_MKDIR: // make directory
      conns[id]->doMKD(request.requestData());
      break;
  }
  return true;
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

void SiteLogic::addRecentList(const std::shared_ptr<SiteRace> & sr) {
  for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  recentlylistedraces.push_back(sr);
}

bool SiteLogic::wasRecentlyListed(const std::shared_ptr<SiteRace> & sr) const {
  for (std::list<std::shared_ptr<SiteRace>>::const_iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      return true;
    }
  }
  return false;
}

void SiteLogic::refreshChangePath(int id, const std::shared_ptr<SiteRace> & race, bool refresh) {
  if (race->isAborted()) {
    return;
  }
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
  bool transfersslclient = connstatetracker[id].getTransferSSLClient();
  bool sscnmode = conns[id]->getSSCNMode();
  bool passive = connstatetracker[id].getTransferPassive();
  Status status = connstatetracker[id].getTransferMonitor()->getStatus();
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
    if (site->getTLSMode() != TLSMode::NONE || conns[id]->getProtectedMode() == PROT_P) {
      conns[id]->doPROTC();
      return;
    }
  }
  if (status != (passive ? TM_STATUS_AWAITING_PASSIVE : TM_STATUS_AWAITING_ACTIVE)) {
    return;
  }
  if (connstatetracker[id].getTransferFXP()) {
    if (transferssl && (!passive || !site->supportsCPSV())) {
      if (transfersslclient && !sscnmode) {
        conns[id]->doSSCN(true);
        return;
      }
      if (!transfersslclient && sscnmode) {
        conns[id]->doSSCN(false);
        return;
      }
    }
  }
  else if (transferssl && sscnmode) {
    conns[id]->doSSCN(false);
    return;
  }
  if (!passive) {
    conns[id]->doPORT(connstatetracker[id].getTransferHost(),
                      connstatetracker[id].getTransferPort());
  }
  else {
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
  requests.push_back(SiteLogicRequest(requestid, REQ_IDLE, site->getBasePath().toString(), site->getMaxIdleTime(), true));
  activateOne();
  return requestid;
}

int SiteLogic::requestAllIdle(const Path & path, int idletime) {
  if (!idletime) {
    idletime = site->getMaxIdleTime();
  }
  int requestid = -1;
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    requestid = requestidcounter++;
    SiteLogicRequest request(requestid, REQ_IDLE, path.toString(), idletime, false);
    request.setConnId(i);
    requests.push_back(request);
  }
  activateAll();
  return requestid;
}

int SiteLogic::requestAllIdle(int idletime) {
  return requestAllIdle(site->getBasePath(), idletime);
}

int SiteLogic::requestMakeDirectory(const std::string & dirname) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_MKDIR, dirname, true));
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

void SiteLogic::abortRace(const std::shared_ptr<SiteRace> & race) {
  if (!race) {
    return;
  }
  race->abort();
  removeRace(race);
}

void SiteLogic::removeRace(const std::shared_ptr<SiteRace> & race) {
  if (!race) {
    return;
  }
  abortTransfers(race);
  for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == race) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  for (std::vector<std::shared_ptr<SiteRace>>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if (*it == race) {
      currentraces.erase(it);
      return;
    }
  }
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].purgeSiteRace(race);
  }
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (conns[i]->currentCommandOwner() == race) {
      conns[i]->resetCurrentCommandOwner();
    }
  }
}

void SiteLogic::abortTransfers(const std::shared_ptr<CommandOwner> & co) {
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    if (connstatetracker[i].isLoggedIn() && connstatetracker[i].isTransferLocked() &&
        connstatetracker[i].getCommandOwner() == co)
    {
      connstatetracker[i].getTransferMonitor()->getTransferStatus()->setAborted();
      disconnectConn(i);
      connectConn(i);
    }
  }
}

FileListData * SiteLogic::getFileListData(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return static_cast<FileListData *>(it->requestData());
    }
  }
  return NULL;
}

std::string SiteLogic::getRawCommandResult(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      std::string ret = *static_cast<std::string *>(it->requestData());
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
  assert(false);
  return false;
}

const std::shared_ptr<Site> & SiteLogic::getSite() const {
  return site;
}

std::shared_ptr<SiteRace> SiteLogic::getRace(const std::string & race) const {
  for (std::vector<std::shared_ptr<SiteRace>>::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) {
      return *it;
    }
  }
  return std::shared_ptr<SiteRace>();
}

std::shared_ptr<SiteRace> SiteLogic::getRace(unsigned int id) const {
  for (std::vector<std::shared_ptr<SiteRace>>::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return std::shared_ptr<SiteRace>();
}

bool SiteLogic::lockDownloadConn(FileList * fl, int * ret, const std::shared_ptr<CommandOwner> & co, TransferMonitor * tm) {
  return lockTransferConn(fl, ret, tm, co, true);
}

bool SiteLogic::lockUploadConn(FileList * fl, int * ret, const std::shared_ptr<CommandOwner> & co, TransferMonitor * tm) {
  return lockTransferConn(fl, ret, tm, co, false);
}

bool SiteLogic::lockTransferConn(FileList * fl, int * ret, TransferMonitor * tm, const std::shared_ptr<CommandOwner> & co, bool isdownload) {
  TransferType type(TransferType::REGULAR);
  if (isdownload) {
    if (!!co) {
      if (co->classType() == COMMANDOWNER_SITERACE) {
        if (std::static_pointer_cast<SiteRace>(co)->isAffil()) {
          type = TransferType::PRE;
        }
        else if (std::static_pointer_cast<SiteRace>(co)->isDone()) {
          type = TransferType::COMPLETE;
        }
      }
      else if (co->classType() == COMMANDOWNER_TRANSFERJOB) {
        type = TransferType::TRANSFERJOB;
      }
    }
  }
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
        if (!getSlot(isdownload, type)) {
          return false;
        }
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
            if (!getSlot(isdownload, type)) {
              return false;
            }
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
    if (!getSlot(isdownload, type)) {
      return false;
    }
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
        disconnectConn(i, true);
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
      disconnectConn(0, true);
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
  ptrack->updateSlots(site->getMaxDown());
}

bool SiteLogic::downloadSlotAvailable(TransferType type) const {
  if (!available) {
    return false;
  }
  int maxlogins = site->getMaxLogins();
  if ((site->getLeaveFreeSlot() && slotsdn + slotsup + 1 >= maxlogins && maxlogins > 1)) {
    return false;
  }
  switch (type) {
    case TransferType::REGULAR:
      return slotsdn < static_cast<int>(site->getMaxDown());
    case TransferType::PRE:
      return slotsdn < static_cast<int>(site->getMaxDownPre());
    case TransferType::COMPLETE:
      return slotsdn < static_cast<int>(site->getMaxDownComplete());
    case TransferType::TRANSFERJOB:
      return slotsdn < static_cast<int>(site->getMaxDownTransferJob());
  }
  return false;
}

bool SiteLogic::uploadSlotAvailable() const {
  if (!available) {
    return false;
  }
  int maxlogins = site->getMaxLogins();
  if ((site->getLeaveFreeSlot() && slotsdn + slotsup + 1 >= maxlogins && maxlogins > 1)) {
    return false;
  }
  return slotsup < static_cast<int>(site->getMaxUp());
}

int SiteLogic::slotsAvailable() const {
  return available;
}

void SiteLogic::transferComplete(int id, bool isdownload) {
  if (isdownload) {
    slotsdn--;
  }
  else {
    slotsup--;
  }
  available++;
  conns[id]->unsetRawBufferCallback();
}

bool SiteLogic::getSlot(bool isdownload, TransferType type) {
  if (isdownload) {
    if ((type == TransferType::REGULAR && slotsdn >= static_cast<int>(site->getMaxDown())) ||
        (type == TransferType::PRE && slotsdn >= static_cast<int>(site->getMaxDownPre())) ||
        (type == TransferType::COMPLETE && slotsdn >= static_cast<int>(site->getMaxDownComplete())) ||
        (type == TransferType::TRANSFERJOB && slotsdn >= static_cast<int>(site->getMaxDownTransferJob())))
    {
      return false;
    }
    slotsdn++;
  }
  else {
    if (slotsup >= static_cast<int>(site->getMaxUp())) {
      return false;
    }
    slotsup++;
  }
  available--;
  return true;
}

void SiteLogic::pushPotential(int score, const std::string & file, const std::shared_ptr<SiteLogic> & dst) {
  ptrack->pushPotential(score, file, dst, dst->getSite()->getMaxUp());
}

bool SiteLogic::potentialCheck(int score) {
  int max = ptrack->getMaxAvailablePotential();
  if (score >= max * 0.65) {
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
  rawcommandrawbuf->rename(site->getName());
}

int SiteLogic::getCurrLogins() const {
  return loggedin;
}

int SiteLogic::getCurrDown() const {
  return slotsdn;
}

int SiteLogic::getCurrUp() const {
  return slotsup;
}

int SiteLogic::getCleanlyClosedConnectionsCount() const {
  int count = 0;
  for (int i = 0; i < (int)conns.size(); ++i) {
    if (conns[i]->isCleanlyClosed()) {
      count++;
    }
  }
  return count;
}

void SiteLogic::connectConn(int id) {
  if (!conns[id]->isConnected()) {
    connstatetracker[id].resetIdleTime();
    conns[id]->login();
  }
}

void SiteLogic::disconnectConn(int id, bool hard) {
  connstatetracker[id].resetIdleTime();
  cleanupConnection(id);
  if (conns[id]->isConnected()) {
    if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isQuitting()) {
      loggedin--;
      available--;
    }
    if (!hard && connstatetracker[id].isLoggedIn() && !conns[id]->isProcessing()) {
      conns[id]->doQUIT();
      connstatetracker[id].setQuitting();
    }
    else {
      conns[id]->disconnect();
      connstatetracker[id].setDisconnected();
    }
  }
}

void SiteLogic::cleanupConnection(int id) {
  while (connstatetracker[id].isListOrTransferLocked()) {
    reportTransferErrorAndFinish(id, 3);
  }
  if (connstatetracker[id].hasRequest()) {
    setRequestReady(id, NULL, false);
  }
  for (int i = 0; i < (int)conns.size(); ++i) {
    if (i != id && conns[i]->isConnected()) {
      return;
    }
  }
  bool erased = true;
  while (erased) {
    erased = false;
    for (std::list<SiteLogicRequest>::iterator it = requests.begin(); it != requests.end(); it++)
    {
      if (it->connId() != -1 && it->connId() != id) {
        continue;
      }
      connstatetracker[id].setRequest(*it);
      requests.erase(it);
      setRequestReady(id, NULL, false);
      erased = true;
      break;
    }
  }
}

void SiteLogic::finishTransferGracefully(int id) {
  assert(connstatetracker[id].hasTransfer() &&
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

void SiteLogic::listCompleted(int id, int storeid, FileList * fl, const std::shared_ptr<CommandOwner> & co) {
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

void SiteLogic::raceGlobalComplete(const std::shared_ptr<SiteRace> & sr) {
  removeRace(sr);
  bool stillactive = !currentraces.empty();
  for (std::list<std::shared_ptr<SiteTransferJob> >::const_iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    if (!(*it)->getTransferJob()->isDone()) {
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

void SiteLogic::raceLocalComplete(const std::shared_ptr<SiteRace> & sr, int uploadslotsleft, bool reportdone) {
  sr->complete(reportdone);
  bool stillactive = false;
  for (unsigned int i = 0; i < currentraces.size(); i++) {
    if (!currentraces[i]->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    int downloadslots = site->getMaxDown();
    if (static_cast<int>(site->getMaxDownComplete()) > downloadslots) {
      downloadslots = site->getMaxDownComplete();
    }
    if (sr->isAffil() && static_cast<int>(site->getMaxDownPre()) > downloadslots) {
      downloadslots = site->getMaxDownPre();
    }
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
    return "IDLE " + std::to_string(idletime) + "s";
  }
  return conns[id]->getStatus();
}

void SiteLogic::preparePassiveTransfer(int id, const std::string & file, bool fxp, bool ssl, bool sslclient) {
  connstatetracker[id].setTransfer(file, fxp, ssl, sslclient);
  initTransfer(id);
}

void SiteLogic::prepareActiveTransfer(int id, const std::string & file, bool fxp, const std::string & host, int port, bool ssl, bool sslclient) {
  connstatetracker[id].setTransfer(file, fxp, host, port, ssl, sslclient);
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
    global->getTransferManager()->getFileList(global->getSiteLogicManager()->getSiteLogic(this), id, hiddenfiles, fl);
  }
}

void SiteLogic::getFileListConn(int id, const std::shared_ptr<CommandOwner> & co, FileList * filelist) {
  assert(filelist != NULL);
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
      connstatetracker[id].getTransferSSLClient() &&
      site->supportsCPSV())
  {
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
  const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
  requestsready.push_back(SiteLogicRequestReady(request->requestType(), request->requestId(), data, status));
  while (requestsready.size() > MAX_REQUEST_READY_QUEUE_SIZE && timesincelastrequestready > MAX_REQUEST_READY_QUEUE_IDLE_TIME) {
    clearReadyRequest(requestsready.front());
    requestsready.pop_front();
  }
  timesincelastrequestready = 0;
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
      delete static_cast<FileListData *>(data);
    }
    else if (request.getType() == REQ_RAW) {
      delete static_cast<std::string *>(data);
    }
  }
}
