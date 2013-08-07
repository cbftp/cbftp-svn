#include "sitelogic.h"

SiteLogic::SiteLogic(std::string sitename) {
  requestidcounter = 0;
  site = global->getSiteManager()->getSite(sitename);
  max_slots_dn = slots_dn = site->getMaxDown();
  max_slots_up = slots_up = site->getMaxUp();
  ptrack = new PotentialTracker(slots_dn);
  available = 0;
  loggedin = 0;
  wantedloggedin = 0;
  poke = false;
  rawbuf = new RawBuffer(site->getName());
  int logins = site->getMaxLogins();
  global->getTickPoke()->startPoke(this, 50, 0);
  for (int i = 0; i < logins; i++) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, i));
  }
}

SiteLogic::~SiteLogic() {
  global->getTickPoke()->stopPoke(this, 0);
  delete ptrack;
}

void SiteLogic::activate() {
  wantedloggedin = conns.size();
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (connstatetracker[i].isDisconnected()) {
      conns[i]->login();
    }
    else if (connstatetracker[i].isReady()) {
      handleConnection(i, false);
    }
    else if (connstatetracker[i].isIdle()) {
      connstatetracker[i].setReady();
      handleConnection(i, false);
    }
  }
}

void SiteLogic::addRace(Race * enginerace, std::string section, std::string release) {
  SiteRace * race = new SiteRace(enginerace, site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activate();
}

void SiteLogic::tick(int message) {
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(50);
    if (connstatetracker[i].hasReleasedCommand()) {
      DelayedCommand eventcommand = connstatetracker[i].getCommand();
      std::string event = eventcommand.getCommand();
      if (event == "refreshchangepath") {
        if (!conns[i]->isProcessing() && connstatetracker[i].isReady()) {
          SiteRace * race = (SiteRace *) eventcommand.getArg();
          refreshChangePath(i, race, true);
        }
      }
      else if (event == "userpass") {
        conns[i]->doUSER(false);
      }
      else if (event == "reconnect") {
        conns[i]->reconnect();
      }
      else if (event == "quit") {
        conns[i]->doQUIT();
        connstatetracker[i].setDisconnected();
        loggedin--;
        available--;
        if (wantedloggedin > loggedin) {
          wantedloggedin = loggedin;
        }
      }
    }
  }
}

void SiteLogic::connectFailed(int id) {

}

void SiteLogic::userDenied(int id) {

}

void SiteLogic::loginKillFailed(int id) {

}

void SiteLogic::passDenied(int id) {

}

void SiteLogic::TLSFailed(int id) {

}

void SiteLogic::listRefreshed(int id) {
  SiteRace * sr = conns[id]->currentSiteRace();
  if (sr != NULL) {
    sr->updateNumFilesUploaded();
    sr->addNewDirectories();
  }
  if (connstatetracker[id].hasTransfer()) {
    initTransfer(id);
    return;
  }
  std::list<SiteLogicRequest>::iterator it;
  for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
    if (it->connId() == id) {
      requestsready.push_back(SiteLogicRequestReady(it->requestId(), conns[id]->currentFileList()));
      requestsinprogress.erase(it);
      global->getUICommunicator()->backendPush();
      break;
    }
  }
  if (sr != NULL) {
    ((EngineBase *)global->getEngine())->someRaceFileListRefreshed();
  }
  handleConnection(id, true);
}

void SiteLogic::unexpectedResponse(int id) {

}

void SiteLogic::commandSuccess(int id) {
  int state = conns[id]->getState();
  std::list<SiteLogicRequest>::iterator it;
  switch (state) {
    case 5: // PASS, logged in
      loggedin++;
      available++;
      connstatetracker[id].setIdle();
      break;
    case 8: // PROT P
    case 9: // PROT C
      if (connstatetracker[id].hasTransfer()) {
        initTransfer(id);
        return;
      }
      break;
    case 13: // PORT
      if (connstatetracker[id].hasTransfer()) {
        if (connstatetracker[id].getTransferDownload()) {
          conns[id]->doRETR(connstatetracker[id].getTransferFile());
          return;
        }
        else {
          conns[id]->doSTOR(connstatetracker[id].getTransferFile());
          return;
        }
      }
      break;
    case 14: // CWD
      if (conns[id]->hasMKDCWDTarget()) {
        if (conns[id]->getCurrentPath() == conns[id]->getMKDCWDTargetSection() +
            "/" + conns[id]->getMKDCWDTargetPath()) {
          conns[id]->finishMKDCWDTarget();
        }
      }
      if (connstatetracker[id].hasTransfer()) {
        initTransfer(id);
        return;
      }
      for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
        if (it->connId() == id) {
          if (it->requestType() == 0) {
            conns[id]->doSTAT();
            return;
          }
          break;
        }
      }
      break;
    case 15: // MKD
      if (conns[id]->hasMKDCWDTarget()) {
        std::string targetcwdsect = conns[id]->getMKDCWDTargetSection();
        std::string targetcwdpath = conns[id]->getMKDCWDTargetPath();
        std::list<std::string> * subdirs = conns[id]->getMKDSubdirs();
        if (conns[id]->getTargetPath() == targetcwdsect + "/" + targetcwdpath) {
          conns[id]->doCWD(conns[id]->getTargetPath());
          return;
        }
        else if (subdirs->size() > 0) {
          std::string sub = subdirs->front();
          subdirs->pop_front();
          conns[id]->doMKD(conns[id]->getTargetPath() + "/" + sub);
          return;
        }
      }
      break;

    case 16: // PRET RETR
    case 17: // PRET STOR
      if (connstatetracker[id].hasTransfer()) {
        if (connstatetracker[id].getTransferPassive()) {
          if (!connstatetracker[id].getTransferSSL()) {
            conns[id]->doPASV();
          }
          else {
            conns[id]->doCPSV();
          }
          return;
        }
      }
      break;
    case 18: // RETR started
      // no action yet, maybe for stats later on
      /*if (connstatetracker[id].isReady()) {
        std::cout << "NOT BUSY!>" << site->getName() << id << " " << connstatetracker[id].getTransferFile()<< connstatetracker[id].isIdle() << std::endl;
        sleep(5);
      }*/
      return;
    case 19: // RETR complete
      if (connstatetracker[id].hasTransfer()) {
        connstatetracker[id].getTransferMonitor()->sourceComplete();
        transferComplete(true);
        connstatetracker[id].finishTransfer();
      }
      break;
    case 20: // STOR started
      // no action yet, maybe for stats later on
      /*if (connstatetracker[id].isReady()) {
        std::cout << "NOT BUSY!>" << site->getName() << id << " " << connstatetracker[id].getTransferFile()<< connstatetracker[id].isIdle() << std::endl;
        sleep(5);
      }*/
      return;
    case 21: // STOR complete
      if (connstatetracker[id].hasTransfer()) {
        connstatetracker[id].getTransferMonitor()->targetComplete();
        transferComplete(false);
        connstatetracker[id].finishTransfer();
      }
      break;
    case 22: // ABOR
      connstatetracker[id].setIdle();
      break;
  }
  handleConnection(id, false);
}

void SiteLogic::commandFail(int id) {
  int state = conns[id]->getState();
  std::string targetcwdsect;
  std::string targetcwdpath;
  std::list<std::string> * subdirs;
  std::string file;
  switch (state) {
    case 14: // cwd fail
      if (conns[id]->hasMKDCWDTarget()) {
        conns[id]->doMKD(conns[id]->getTargetPath());
        return;
      }
      break;
    case 15: // mkd fail
      targetcwdsect = conns[id]->getMKDCWDTargetSection();
      targetcwdpath = conns[id]->getMKDCWDTargetPath();
      subdirs = conns[id]->getMKDSubdirs();
      if (conns[id]->getTargetPath() == targetcwdsect + "/" + targetcwdpath) {
        if (subdirs->size() > 0) {
          std::string sub = subdirs->front();
          subdirs->pop_front();
          conns[id]->doMKD(targetcwdsect + "/" + sub);
          return;
        }
        else {
          // failed mkd and no target subdirs. shouldn't happen.
        }
      }
      else {
        // cwdmkd failed.
      }
      break;
    case 16: // PRET RETR fail
      handleTransferFail(id, true, 0);
      return;
    case 17: // PRET STOR fail
      handleTransferFail(id, false, 0);
      return;
    case 18: // RETR fail
      handleTransferFail(id, true, 1);
      return;
    case 19: // RETR post fail
      handleTransferFail(id, true, 2);
      return;
    case 20: // STOR fail
      handleTransferFail(id, false, 1);
      return;
    case 21: // STOR post fail
      handleTransferFail(id, false, 2);
      return;
  }
  // default handling: reconnect
  conns[id]->reconnect();
}

void SiteLogic::handleTransferFail(int id, bool download, int err) {
  if (connstatetracker[id].hasTransfer()) {
    if (download) {
      connstatetracker[id].getTransferMonitor()->sourceError(err);
    }
    else {
      connstatetracker[id].getTransferMonitor()->targetError(err);
    }
    transferComplete(download);
    connstatetracker[id].finishTransfer();
  }
  connstatetracker[id].setIdle();
  if (err == 1) {
    conns[id]->abortTransfer();
  }
  else {
    handleConnection(id, false);
  }
}

void SiteLogic::gotPath(int id, std::string path) {

}

void SiteLogic::rawCommandResultRetrieved(int id, std::string result) {
  rawbuf->write(result);
  std::list<SiteLogicRequest>::iterator it;
  for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
    if (it->connId() == id) {
      requestsinprogress.erase(it);
      global->getUICommunicator()->backendPush();
      break;
    }
  }
  connstatetracker[id].setIdle();
  handleConnection(id, true);
}

void SiteLogic::gotPassiveAddress(int id, std::string result) {
  if (connstatetracker[id].hasTransfer()) {
    connstatetracker[id].getTransferMonitor()->passiveReady(result);
  }
  else {
    handleConnection(id, false);
  }
}

void SiteLogic::timedout(int id) {

}

void SiteLogic::requestSelect() {
  if (loggedin == 0) {
    if (wantedloggedin == 0) {
      wantedloggedin++;
    }
    conns[0]->login();
  }
  else {
    int conn = -1;
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isIdle()) {
        conn = i;
        break;
      }
    }
    if (conn < 0) {
      for (unsigned int i = 0; i < conns.size(); i++) {
        if (connstatetracker[i].isReady()) {
          conn = i;
          break;
        }
      }
    }
    if (conn >= 0) {
      handleRequest(conn);
    }
    else if (loggedin < conns.size()) {
      wantedloggedin++;
      for (unsigned int i = 0; i < conns.size(); i++) {
        if (connstatetracker[i].isDisconnected()) {
          conns[i]->login();
          break;
        }
      }
    }
  }
}

void SiteLogic::handleConnection(int id, bool backfromrefresh) {
  if (!connstatetracker[id].isReady()) {
    return;
  }
  if (loggedin > wantedloggedin) {
    connstatetracker[id].setDisconnected();
    conns[id]->doQUIT();
    available--;
    loggedin--;
    return;
  }
  SiteRace * race = NULL;
  if (requests.size() > 0) {
    handleRequest(id);
    return;
  }
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
    bool goodpath = currentpath.find(race->getPath()) != std::string::npos;
    if (goodpath && !refresh) {
      connstatetracker[id].setIdle();
      return;
    }
    if (backfromrefresh) {
      connstatetracker[id].setReady();
      connstatetracker[id].delayedCommand("refreshchangepath", SLEEPDELAY, (void *) race);
      return;
    }
    if (goodpath) {
      conns[id]->doSTAT(race, race->getFileListForFullPath(currentpath));
      return;
    }
    else {
      refreshChangePath(id, race, refresh);
    }
  }
  else {
    connstatetracker[id].setIdle();
    if (site->getMaxIdleTime()) {
      connstatetracker[id].delayedCommand("quit", site->getMaxIdleTime() * 1000);
    }
  }
}

void SiteLogic::handleRequest(int id) {
  connstatetracker[id].setReady();
  requests.front().setConnId(id);
  if (requests.front().requestType() == 0) {
    std::string targetpath = requests.front().requestData();
    if (conns[id]->getCurrentPath() == targetpath) {
      conns[id]->doSTAT();
    }
    else {
      conns[id]->doCWD(targetpath);
    }
  }
  else {
    rawbuf->writeLine(requests.front().requestData());
    conns[id]->doRaw(requests.front().requestData());
  }
  requestsinprogress.push_back(requests.front());
  requests.pop_front();
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

bool SiteLogic::wasRecentlyListed(SiteRace * sr) {
  for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
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
  if (appendsubpath.length() > 0) {
    appendsubpath = "/" + subpath;
  }
  std::string targetpath = race->getPath() + appendsubpath;
  if (targetpath != currentpath) {
    conns[id]->setMKDCWDTarget(race->getSection(), race->getRelease() + appendsubpath);
    conns[id]->doCWD(targetpath);
  }
  else {
    if (refresh) {
      conns[id]->doSTAT(race, race->getFileListForFullPath(currentpath));
    }
    else {
      connstatetracker[id].setIdle();
    }
  }
}

void SiteLogic::initTransfer(int id) {
  std::string transferpath = connstatetracker[id].getTransferFileList()->getPath();
  bool transferssl = connstatetracker[id].getTransferSSL();
  if (conns[id]->getCurrentPath() != transferpath) {
    conns[id]->doCWD(transferpath);
    return;
  }
  if (transferssl && !conns[id]->getProtectedMode()) {
    conns[id]->doPROTP();
    return;
  }
  else if (!transferssl && conns[id]->getProtectedMode()) {
    conns[id]->doPROTC();
    return;
  }
  if (!connstatetracker[id].getTransferPassive()) {
    conns[id]->doPORT(connstatetracker[id].getTransferAddr());
  }
  else {
    if (site->needsPRET()) {
      if (connstatetracker[id].getTransferDownload()) {
        conns[id]->doPRETRETR(connstatetracker[id].getTransferFile());
      }
      else {
        conns[id]->doPRETSTOR(connstatetracker[id].getTransferFile());
      }
    }
    else {
      if (!transferssl) {
        conns[id]->doPASV();
      }
      else {
        conns[id]->doCPSV();
      }
    }
  }
}

int SiteLogic::requestFileList(std::string path) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, 0, path));
  requestSelect();
  return requestid;
}

int SiteLogic::requestRawCommand(std::string command) {
  int requestid = requestidcounter++;
  requests.push_back(SiteLogicRequest(requestid, 1, command));
  requestSelect();
  return requestid;
}

bool SiteLogic::requestReady(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return true;
    }
  }
  return false;
}

FileList * SiteLogic::getFileList(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
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

void SiteLogic::finishRequest(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      requestsready.erase(it);
      return;
    }
  }
  std::list<SiteLogicRequest>::iterator it2;
  for (it2 = requestsinprogress.begin(); it2 != requestsinprogress.end(); it2++) {
    if (it->requestId() == requestid) {
      requestsinprogress.erase(it2);
      return;
    }
  }
}

Site * SiteLogic::getSite() {
  return site;
}

SiteRace * SiteLogic::getRace(std::string race) {
  for (std::vector<SiteRace *>::iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) return *it;
  }
  return NULL;
}

bool SiteLogic::lockDownloadConn(FileList * fl, std::string file, int * ret) {
  bool locked = getReadyConn(fl, file, ret, true, true);
  if (!locked) return false;
  connstatetracker[*ret].lockForTransfer(true);
  return true;
}

bool SiteLogic::lockUploadConn(FileList * fl, std::string file, int * ret) {
  bool locked = getReadyConn(fl, file, ret, true, false);
  if (!locked) return false;
  connstatetracker[*ret].lockForTransfer(false);
  return true;
}

bool SiteLogic::getReadyConn(FileList * fl, int * ret) {
  return getReadyConn(fl, "", ret, false, false);
}

bool SiteLogic::getReadyConn(FileList * fl, std::string file, int * ret, bool istransfer, bool isdownload) {
  int lastreadyid = -1;
  bool foundreadythread = false;
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isIdle()) {
      foundreadythread = true;
      lastreadyid = i;
      if (conns[i]->getTargetPath().compare(fl->getPath()) == 0) {
        if (istransfer) {
          if (!getSlot(isdownload)) return false;
        }
        connstatetracker[i].setBusy();
        if (!conns[i]->isProcessing()) {
          conns[i]->doCWD(fl->getPath());
        }
        *ret = i;
        return true;
      }
    }
  }
  if (!foundreadythread) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isReady()) {
        foundreadythread = true;
        lastreadyid = i;
        if (conns[i]->getTargetPath().compare(fl->getPath()) == 0) {
          if (istransfer) {
            if (!getSlot(isdownload)) return false;
          }
          connstatetracker[i].setBusy();
          if (!conns[i]->isProcessing()) {
            conns[i]->doCWD(fl->getPath());
          }
          *ret = i;
          return true;
        }
      }
    }
  }
  if (foundreadythread) {
    if (istransfer) {
      if (!getSlot(isdownload)) return false;
    }
    connstatetracker[lastreadyid].setBusy();
    if (!conns[lastreadyid]->isProcessing()) {
      conns[lastreadyid]->doCWD(fl->getPath());
    }
    *ret = lastreadyid;
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
  connstatetracker[id].setIdle();
  if (!conns[id]->isProcessing()) {
    handleConnection(id, false);
  }
}

void SiteLogic::setNumConnections(unsigned int num) {
  while (num < conns.size()) {
    bool success = false;
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isDisconnected()) {
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
      if (connstatetracker[i].isIdle()) {
        conns[i]->doQUIT();
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
  if (max_slots_up < site->getMaxUp()) {
    slots_up += site->getMaxUp() - max_slots_up;
    max_slots_up = site->getMaxUp();
  }
  if (max_slots_dn < site->getMaxDown()) {
    slots_dn += site->getMaxDown() - max_slots_dn;
    max_slots_dn = site->getMaxDown();
  }
  if (max_slots_up > site->getMaxUp()) {
    slots_up -= (max_slots_up - site->getMaxUp());
    max_slots_up = site->getMaxUp();
  }
  if (max_slots_dn > site->getMaxDown()) {
    slots_dn -= (max_slots_dn - site->getMaxDown());
    max_slots_dn = site->getMaxDown();
  }
}

bool SiteLogic::downloadSlotAvailable() {
  return (available > 0 && slots_dn > 0);
}

bool SiteLogic::uploadSlotAvailable() {
  return (available > 0 && slots_up > 0);
}

void SiteLogic::transferComplete(bool isdownload) {
  if (isdownload) {
    slots_dn++;
  }
  else slots_up++;
  available++;
}

bool SiteLogic::getSlot(bool isdownload) {
  if (isdownload) {
    if (slots_dn <= 0) {
      return false;
    }
    slots_dn--;
  } else {
    if (slots_up <= 0) {
      return false;
    }
    slots_up--;
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

int SiteLogic::getCurrLogins() {
  return loggedin;
}

int SiteLogic::getCurrDown() {
  return site->getMaxDown() - slots_dn;
}

int SiteLogic::getCurrUp() {
  return site->getMaxUp() - slots_up;
}

void SiteLogic::connectConn(int id) {
  if (connstatetracker[id].isDisconnected()) {
    if (wantedloggedin < site->getMaxLogins()) {
      wantedloggedin++;
    }
    connstatetracker[id].setIdle();
    conns[id]->login();
  }
}

void SiteLogic::disconnectConn(int id) {
  if (!connstatetracker[id].isDisconnected()) {
    if (wantedloggedin > 0) {
      wantedloggedin--;
    }
    connstatetracker[id].setDisconnected();
    conns[id]->doQUIT();
    available--;
    loggedin--;
  }
}

void SiteLogic::issueRawCommand(unsigned int id, std::string command) {
  connectConn(id);
  rawbuf->writeLine(command);
  conns[id]->doRaw(command);
}

RawBuffer * SiteLogic::getRawCommandBuffer() {
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
      if (connstatetracker[i].isIdle()) {
        connstatetracker[i].setDisconnected();
        conns[i]->doQUIT();
        loggedin--;
        available--;
      }
    }
    wantedloggedin = 0;
  }
}

void SiteLogic::raceLocalComplete(SiteRace * sr) {
  sr->complete();
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
      if (connstatetracker[i].isIdle()) {
        connstatetracker[i].setDisconnected();
        conns[i]->doQUIT();
        loggedin--;
        available--;
        killnum--;
      }
    }
    wantedloggedin = site->getMaxDown();
  }
}

std::vector<FTPConn *> * SiteLogic::getConns() {
  return &conns;
}

FTPConn * SiteLogic::getConn(int id) {
  std::vector<FTPConn *>:: iterator it;
  for (it = conns.begin(); it != conns.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return NULL;
}

std::string SiteLogic::getStatus(int id) {
  if (connstatetracker[id].isIdle()) {
    int idletime = connstatetracker[id].getTimePassed()/1000;
    return "IDLE " + global->int2Str(idletime) + "s";
  }
  return conns[id]->getStatus();
}

void SiteLogic::preparePassiveDownload(int id, TransferMonitorBase * tmb, FileList * fls, std::string file, bool ssl) {
  connstatetracker[id].setTransfer(tmb, fls, file, true, true, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::preparePassiveUpload(int id, TransferMonitorBase * tmb, FileList * fls, std::string file, bool ssl) {
  connstatetracker[id].setTransfer(tmb, fls, file, false, true, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::passiveDownload(int id) {
  conns[id]->doRETR(connstatetracker[id].getTransferFile());
}

void SiteLogic::passiveUpload(int id) {
  conns[id]->doSTOR(connstatetracker[id].getTransferFile());
}

void SiteLogic::activeDownload(int id, TransferMonitorBase * tmb, FileList * fls, std::string file, std::string addr, bool ssl) {
  connstatetracker[id].setTransfer(tmb, fls, file, true, false, addr, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::activeUpload(int id, TransferMonitorBase * tmb, FileList * fls, std::string file, std::string addr, bool ssl) {
  connstatetracker[id].setTransfer(tmb, fls, file, false, false, addr, ssl);
  if (!conns[id]->isProcessing()) {
    initTransfer(id);
  }
}

void SiteLogic::abortTransfer(int id) {
  if (connstatetracker[id].hasTransfer()) {
    transferComplete(connstatetracker[id].getTransferDownload());
    connstatetracker[id].abortTransfer();
  }
}
