#include "sitelogic.h"

#include "sitemanager.h"
#include "ftpconn.h"
#include "filelist.h"
#include "siterace.h"
#include "site.h"
#include "race.h"
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

SiteLogic::SiteLogic(std::string sitename) {
  requestidcounter = 0;
  site = global->getSiteManager()->getSite(sitename);
  maxslotsdn = slotsdn = site->getMaxDown();
  maxslotsup = slotsup = site->getMaxUp();
  ptrack = new PotentialTracker(slotsdn);
  available = 0;
  loggedin = 0;
  wantedloggedin = 0;
  poke = false;
  rawbuf = new RawBuffer(site->getName());
  int logins = site->getMaxLogins();
  global->getTickPoke()->startPoke(this, "SiteLogic", 50, 0);
  for (int i = 0; i < logins; i++) {
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
  wantedloggedin = conns.size();
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (!conns[i]->isConnected()) {
      conns[i]->login();
    }
    else {
      handleConnection(i, false);
    }
  }
}

void SiteLogic::addRace(Race * enginerace, std::string section, std::string release) {
  SiteRace * race = new SiteRace(enginerace, site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activateAll();
}

void SiteLogic::addTransferJob(TransferJob * tj) {
  transferjobs.push_back(tj);
  activateOne();
}

void SiteLogic::tick(int message) {
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(50);
    if (connstatetracker[i].getCommand().isReleased()) {
      if (connstatetracker[i].isLocked() || conns[i]->isProcessing()) {
        *(int*)0=0; // crash on purpose
      }
      DelayedCommand eventcommand = connstatetracker[i].getCommand();
      connstatetracker[i].getCommand().reset();
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
        if (conns[i]->isConnected()) {
          connQuit(i);
        }
        if (wantedloggedin > loggedin) {
          wantedloggedin = loggedin;
        }
      }
    }
  }
}

void SiteLogic::connectFailed(int id) {
  connstatetracker[id].setDisconnected();
}

void SiteLogic::userDenied(int id) {
  connQuit(id);
}

void SiteLogic::userDeniedSiteFull(int id) {
  connstatetracker[id].delayedCommand("reconnect", SLEEPDELAY, NULL, true);
}

void SiteLogic::userDeniedSimultaneousLogins(int id) {
  conns[id]->doUSER(true);
}

void SiteLogic::loginKillFailed(int id) {
  connQuit(id);
}

void SiteLogic::passDenied(int id) {
  connQuit(id);
}

void SiteLogic::TLSFailed(int id) {
  connQuit(id);
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
  std::list<SiteLogicRequest>::iterator it;
  for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
    if (it->connId() == id) {
      requestsready.push_back(SiteLogicRequestReady(it->requestId(), conns[id]->currentFileList(), true));
      requestsinprogress.erase(it);
      global->getUIBase()->backendPush();
      break;
    }
  }
  if (currentco != NULL) {
    if (currentco->classType() == COMMANDOWNER_SITERACE) {
      SiteRace * sr = (SiteRace *)currentco;
      global->getEngine()->raceFileListRefreshed(this, sr->getRace());
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
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_FILELIST) {
            getFileListConn(id);
            return;
          }
          break;
        }
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
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_WIPE_RECURSIVE || it->requestType() == REQ_WIPE) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, true));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
        }
      }
      break;
    case STATE_DELE:
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_DEL) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, true));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
          else if (it->requestType() == REQ_DEL_RECURSIVE) {
            if (connstatetracker[id].getRecursiveLogic()->isActive()) {
              handleRecursiveLogic(id);
              return;
            }
            else {
              requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, true));
              requestsinprogress.erase(it);
              global->getUIBase()->backendPush();
              break;
            }
          }
        }
      }
      break;
    case STATE_NUKE:
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_NUKE) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, true));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
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
        handleTransferFail(id, 3);
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
        handleTransferFail(id, 3);
        return;
      }
      if (connstatetracker[id].hasTransfer()) {
        handleFail(id);
        return;
      }
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_FILELIST) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, false));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            handleConnection(id, false);
            return;
          }
          break;
        }
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
      break;
    case STATE_PRET_RETR:
      handleTransferFail(id, CST_DOWNLOAD, 0);
      return;
    case STATE_PRET_STOR:
      handleTransferFail(id, CST_UPLOAD, 0);
      return;
    case STATE_RETR:
      handleTransferFail(id, CST_DOWNLOAD, 1);
      return;
    case STATE_RETR_COMPLETE:
      handleTransferFail(id, CST_DOWNLOAD, 2);
      return;
    case STATE_STOR:
      handleTransferFail(id, CST_UPLOAD, 1);
      return;
    case STATE_STOR_COMPLETE:
      handleTransferFail(id, CST_UPLOAD, 2);
      return;
    case STATE_WIPE:
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_WIPE_RECURSIVE || it->requestType() == REQ_WIPE) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, false));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
        }
      }
      handleConnection(id, false);
      return;
    case STATE_DELE:
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_DEL_RECURSIVE || it->requestType() == REQ_DEL) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, false));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
        }
      }
      handleConnection(id, false);
      return;
    case STATE_NUKE:
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == REQ_NUKE) {
            requestsready.push_back(SiteLogicRequestReady(it->requestId(), NULL, false));
            requestsinprogress.erase(it);
            global->getUIBase()->backendPush();
            break;
          }
        }
      }
      handleConnection(id, false);
      return;
    case STATE_LIST:
      handleTransferFail(id, CST_LIST, 1);
      return;
    case STATE_PRET_LIST:
      handleTransferFail(id, CST_LIST, 0);
      return;
    case STATE_LIST_COMPLETE:
      handleTransferFail(id, CST_LIST, 2);
      return;
  }
  // default handling: reconnect
  disconnected(id);
  conns[id]->reconnect();
}

void SiteLogic::handleFail(int id) {
  if (connstatetracker[id].transferInitialized()) {
    handleTransferFail(id, 0);
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
        global->int2Str(err) + ") without having a transfer, shouldn't happen!");
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
            global->int2Str(err) + ") without having a transfer, shouldn't happen!");
        break;
      case CST_UPLOAD:
        global->getEventLog()->log("SiteLogic", "BUG: Returned failed upload (code " +
            global->int2Str(err) + ") without having a transfer, shouldn't happen!");
        break;
      case CST_LIST:
        global->getEventLog()->log("SiteLogic", "BUG: Returned failed LIST (code " +
            global->int2Str(err) + ") without having a transfer, shouldn't happen!");
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
        connstatetracker[id].getTransferMonitor()->sourceError(err);
        transferComplete(true);
        break;
      case CST_LIST:
        connstatetracker[id].getTransferMonitor()->sourceError(err);
        break;
      case CST_UPLOAD:
        connstatetracker[id].getTransferMonitor()->targetError(err);
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
  std::list<SiteLogicRequest>::iterator it;
  for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
    if (it->connId() == id) {
      requestsinprogress.erase(it);
      global->getUIBase()->backendPush();
      break;
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
  while (connstatetracker[id].hasTransfer()) {
    reportTransferErrorAndFinish(id, 3);
  }
  if (connstatetracker[id].isLoggedIn()) {
    loggedin--;
    available--;
  }
  connstatetracker[id].setDisconnected();
}

void SiteLogic::connQuit(int id) {
  if (connstatetracker[id].isLoggedIn()) {
    available--;
    loggedin--;
  }
  connstatetracker[id].setDisconnected();
  conns[id]->doQUIT();
}

void SiteLogic::activateOne() {
  if (loggedin == 0) {
    if (wantedloggedin == 0) {
      wantedloggedin++;
    }
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
      wantedloggedin++;
      for (unsigned int i = 0; i < conns.size(); i++) {
        if (!conns[i]->isConnected()) {
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
    wantedloggedin = connected;
    for (unsigned int i = 0; i < conns.size() && lefttoconnect > 0; i++) {
      if (!conns[i]->isConnected()) {
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
  if (loggedin > wantedloggedin) {
    connQuit(id);
    return;
  }
  if (requests.size() > 0) {
    if (handleRequest(id)) {
      return;
    }
  }
  TransferJob * targetjob = NULL;
  for (std::list<TransferJob *>::iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    TransferJob * tj = *it;
    if (!tj->isDone()) {
      if (tj->wantsList(this)) {
        FileList * fl = tj->getListTarget(this);
        std::string path = fl->getPath();
        if (path != conns[id]->getCurrentPath()) {
          conns[id]->doCWD(path);
          return;
        }
        getFileListConn(id, tj, fl);
        return;
      }
      if (!tj->isInitialized()) {
        targetjob = tj;
        continue;
      }
      int type = tj->getType();
      if (targetjob == NULL &&
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
  if (targetjob != NULL) {
    connstatetracker[id].delayedCommand("handle", SLEEPDELAY);
    global->getEngine()->transferJobActionRequest(targetjob);
    return;
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
      if (!races[i]->getRace()->isDone()) {
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
  requestsinprogress.push_back(*it);
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
  requests.push_back(SiteLogicRequest(requestid, REQ_FILELIST, path));
  activateOne();
  return requestid;
}

int SiteLogic::requestRawCommand(std::string command) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_RAW, command));
  activateOne();
  return requestid;
}

int SiteLogic::requestWipe(std::string path, bool recursive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE_RECURSIVE, path));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_WIPE, path));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestDelete(std::string path, bool recursive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.push_back(SiteLogicRequest(requestid, REQ_DEL_RECURSIVE, path));
  }
  else {
    requests.push_back(SiteLogicRequest(requestid, REQ_DEL, path));
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestNuke(std::string path, int multiplier, std::string reason) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, REQ_NUKE, path, reason, multiplier));
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

void SiteLogic::abortRace(std::string race) {
  SiteRace * delrace = NULL;
  for (std::vector<SiteRace *>::iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease() == race) {
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
  std::list<SiteLogicRequest>::iterator it2;
  for (it2 = requestsinprogress.begin(); it2 != requestsinprogress.end(); it2++) {
    if (it->requestId() == requestid) {
      bool status = it->requestStatus();
      requestsinprogress.erase(it2);
      return status;
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

bool SiteLogic::lockDownloadConn(std::string path, int * ret) {
  return lockTransferConn(path, ret, true);
}

bool SiteLogic::lockUploadConn(std::string path, int * ret) {
  return lockTransferConn(path, ret, false);
}

bool SiteLogic::lockTransferConn(std::string path, int * ret, bool isdownload) {
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
        connstatetracker[i].lockForTransfer(isdownload);
        return true;
      }
    }
  }
  if (!foundreadythread) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isTransferLocked()) {
        foundreadythread = true;
        lastreadyid = i;
        if (conns[i]->getTargetPath().compare(path) == 0) {
          if (!getSlot(isdownload)) return false;
          *ret = i;
          connstatetracker[i].lockForTransfer(isdownload);
          return true;
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
    connstatetracker[lastreadyid].lockForTransfer(isdownload);
    return true;
  }
  else return false;
}

void SiteLogic::returnConn(int id) {
  if (connstatetracker[id].isLockedForDownload()) {
    transferComplete(true);
  }
  else if (connstatetracker[id].isLockedForUpload()) {
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
        if (connstatetracker[i].isLoggedIn()) {
          loggedin--;
          available--;
        }
        conns[i]->disconnect();
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
      if (conns[0]->isConnected()) {
        if (connstatetracker[0].isLoggedIn()) {
          loggedin--;
          available--;
        }
        conns[0]->disconnect();
      }
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
    if (wantedloggedin < site->getMaxLogins()) {
      wantedloggedin++;
    }
    conns[id]->login();
  }
}

void SiteLogic::disconnectConn(int id) {
  connstatetracker[id].resetIdleTime();
  if (conns[id]->isConnected()) {
    if (wantedloggedin > 0) {
      wantedloggedin--;
    }
    if (connstatetracker[id].isLoggedIn()) {
      connQuit(id);
    }
    else {
      conns[id]->disconnect();
      connstatetracker[id].setDisconnected();
    }
  }
}

void SiteLogic::listCompleted(int id, int storeid) {
  char * data;
  int datalen = global->getLocalStorage()->getStoreContent(storeid, &data);
  conns[id]->parseFileList(data, datalen);
  listRefreshed(id);
  global->getLocalStorage()->purgeStoreContent(storeid);
}

void SiteLogic::issueRawCommand(unsigned int id, std::string command) {
  int requestid = requestidcounter++;
  SiteLogicRequest request(requestid, REQ_RAW, command);
  request.setConnId(id);
  requests.push_back(request);
  if (!conns[id]->isConnected()) {
    connectConn(id);
    return;
  }
  if (!connstatetracker[id].isLocked() && !conns[id]->isProcessing()) {
    handleRequest(id);
  }
}

RawBuffer * SiteLogic::getRawCommandBuffer() const {
  return rawbuf;
}

void SiteLogic::raceGlobalComplete() {
  bool stillactive = false;
  for (unsigned int i = 0; i < races.size(); i++) {
    if (!races[i]->getRace()->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing()) {
        connQuit(i);
      }
    }
    wantedloggedin = 0;
  }
}

void SiteLogic::raceLocalComplete(SiteRace * sr) {
  sr->complete(true);
  bool stillactive = false;
  for (unsigned int i = 0; i < races.size(); i++) {
    if (!races[i]->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    int killnum = site->getMaxLogins() - site->getMaxDown();
    if (killnum < 0) {
      killnum = 0;
    }
    for (unsigned int i = 0; i < conns.size() && killnum > 0; i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing()) {
        connQuit(i);
        killnum--;
      }
    }
    wantedloggedin = site->getMaxDown();
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
    return "IDLE " + global->int2Str(idletime) + "s";
  }
  return conns[id]->getStatus();
}

void SiteLogic::preparePassiveDownload(int id, TransferMonitor * tmb, std::string path, std::string file, bool fxp, bool ssl) {
  connstatetracker[id].setTransfer(tmb, path, file, CST_DOWNLOAD, fxp, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::preparePassiveUpload(int id, TransferMonitor * tmb, std::string path, std::string file, bool fxp, bool ssl) {
  connstatetracker[id].setTransfer(tmb, path, file, CST_UPLOAD, fxp, ssl);
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

void SiteLogic::prepareActiveDownload(int id, TransferMonitor * tmb, std::string path, std::string file, std::string addr, bool ssl) {
  connstatetracker[id].setTransfer(tmb, path, file, CST_DOWNLOAD, addr, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::prepareActiveUpload(int id, TransferMonitor * tmb, std::string path, std::string file, std::string addr, bool ssl) {
  connstatetracker[id].setTransfer(tmb, path, file, CST_UPLOAD, addr, ssl);
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
  if (filelist == NULL) {
    *(int*)0=0; // crash on purpose
  }
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
