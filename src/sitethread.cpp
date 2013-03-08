#include "sitethread.h"

SiteThread::SiteThread(std::string sitename) {
  pthread_mutex_init(&slots, NULL);
  sem_init(&notifysem, 0, 0);
  requestidcounter = 0;
  ftpthreadcom = new FTPThreadCom(&notifysem);
  site = global->getSiteManager()->getSite(sitename);
  max_slots_dn = slots_dn = site->getMaxDown();
  max_slots_up = slots_up = site->getMaxUp();
  ptrack = new PotentialTracker(slots_dn);
  available = 0;
  loggedin = 0;
  wantedloggedin = 0;
  rawbuf = new RawBuffer(site->getName());
  int logins = site->getMaxLogins();
  list_refresh = global->getListRefreshSem();
  global->getTickPoke()->startPoke(&notifysem, 50, 0);
  for (int i = 0; i < logins; i++) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPThread(i, site, ftpthreadcom));
  }
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "SiteThread");
}

SiteThread::~SiteThread() {
  global->getTickPoke()->stopPoke(&notifysem, 0);
  delete ftpthreadcom;
  delete ptrack;
}

void SiteThread::activate() {
  wantedloggedin = conns.size();
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (connstatetracker[i].isDisconnected()) {
      conns[i]->loginAsync();
    }
    else if (connstatetracker[i].isReady()) {
      handleConnection(i);
    }
    else if (connstatetracker[i].isIdle()) {
      connstatetracker[i].setReady();
      handleConnection(i);
    }
  }
}

void SiteThread::addRace(Race * enginerace, std::string section, std::string release) {
  SiteRace * race = new SiteRace(enginerace, site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activate();
}

void * SiteThread::run(void * arg) {
  ((SiteThread *) arg)->runInstance();
  return NULL;
}

void SiteThread::runInstance() {
  SiteRace * race;
  while(1) {
    sem_wait(&notifysem);

    if (global->getTickPoke()->isPoked(&notifysem)) {
      global->getTickPoke()->getMessage(&notifysem);
      for (unsigned int i = 0; i < connstatetracker.size(); i++) {
        connstatetracker[i].timePassed(50);
        if (connstatetracker[i].hasReleasedCommand()) {
          DelayedCommand eventcommand = connstatetracker[i].getCommand();
          std::string event = eventcommand.getCommand();
          if (event == "refresh") {
            race = (SiteRace *) eventcommand.getArg();
            connstatetracker[i].setReady();
            conns[i]->refreshRaceFileListAsync(race);
          }
          else if (event == "userpass") {
            conns[i]->doUSERPASSAsync();
          }
          else if (event == "reconnect") {
            conns[i]->reconnectAsync();
          }
          else if (event == "quit") {
            conns[i]->doQUITAsync();
            connstatetracker[i].setDisconnected();
            loggedin--;
            if (wantedloggedin > loggedin) {
              wantedloggedin = loggedin;
            }
          }
        }
      }
      continue;
    }
    if (!ftpthreadcom->hasCommand()) {
      if (requests.size() > 0) {
        if (loggedin == 0) {
          if (wantedloggedin == 0) {
            wantedloggedin++;
          }
          conns[0]->loginAsync();
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
                conns[i]->loginAsync();
                break;
              }
            }
          }
        }
      }
    }
    else {
      CommandQueueElement * command = ftpthreadcom->getCommand();
      int id = *(int *)command->getArg1();
      int * reqidp;
      int reqid;
      FileList * ret;
      std::string * rets;
      int status;
      std::string reply;
      std::list<SiteThreadRequest>::iterator it;
      bool matchingrequest = false;
      delete (int *)command->getArg1();
      switch(command->getOpcode()) {
        case 0: // login successful / ready to work
          pthread_mutex_lock(&slots);
          loggedin++;
          pthread_mutex_unlock(&slots);
          connstatetracker[id].setIdle();
          handleConnection(id);
          break;
        case 1: // connection failed
          std::cout << "CONNECTION FAILED" << std::endl;
          break;
        case 2: // unknown connect response
          conns[id]->disconnectAsync();
          break;
        case 3: // login user denied
          status = *(int *)command->getArg2();
          delete (int *)command->getArg2();
          reply = std::string((char *)command->getArg3());
          delete (char *)command->getArg3();
          if (status == 530 || status == 550) {
            if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
              conns[id]->disconnectAsync();
              connstatetracker[id].delayedCommand("userpass", SLEEPDELAY);
              break;
            }
            else if (reply.find("simultaneous") != std::string::npos) {
              conns[id]->loginKillAsync();
              break;
            }
          }
          conns[id]->doQUITAsync();
          break;
        case 4: // login password denied
          status = *(int *)command->getArg2();
          delete (int *)command->getArg2();
          reply = std::string((char *)command->getArg3());
          delete (char *)command->getArg3();
          if (status == 530 || status == 550) {
            if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
              conns[id]->disconnectAsync();
              connstatetracker[id].delayedCommand("reconnect", SLEEPDELAY);
            }
            else if (reply.find("simultaneous") != std::string::npos) {
              conns[id]->loginKillAsync();
            }
          }
          break;
        case 5: // TLS request failed
          conns[id]->doQUITAsync();
          break;
        case 6: // connection closed unexpectedly
          std::cout << "Disconnected." << std::endl;
          conns[id]->disconnectAsync();
          break;
        case 7: // login kill failed
          delete (char *)command->getArg3();
          conns[id]->doQUITAsync();
          break;
        case 8: // returned from transfer job
          handleConnection(id);
          break;
        case 10: // file list refreshed in race
          race = (SiteRace *) command->getArg2();
          race->updateNumFilesUploaded();
          race->addNewDirectories();
          int tmpi;
          sem_getvalue(list_refresh, &tmpi);
          if (tmpi == 0) sem_post(list_refresh);
          handleConnection(id, true);
          break;
        case 11: // filelist request retrieved
          reqidp = (int *)command->getArg2();
          reqid = *reqidp;
          delete reqidp;
          ret = (FileList *) command->getArg3();
          matchingrequest = false;
          for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
            if (it->requestId() == reqid) {
              requestsready.push_back(SiteThreadRequestReady(it->requestId(), ret));
              requestsinprogress.erase(it);
              global->getUICommunicator()->backendPush();
              matchingrequest = true;
              break;
            }
          }
          if (!matchingrequest) {
            delete ret;
          }
          handleConnection(id);
          break;
        case 12: // rawcommand result retrieved
          reqidp = (int *)command->getArg2();
          reqid = *reqidp;
          delete reqidp;
          rets = (std::string *) command->getArg3();
          rawbuf->writeLine(std::string(*rets));
          matchingrequest = false;
          for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
            if (it->requestId() == reqid) {
              requestsready.push_back(SiteThreadRequestReady(it->requestId(), rets));
              requestsinprogress.erase(it);
              global->getUICommunicator()->backendPush();
              matchingrequest = true;
              break;
            }
          }
          if (!matchingrequest) {
            delete rets;
          }
          handleConnection(id);
          break;
      }
      ftpthreadcom->commandProcessed();
    }
  }
}

void SiteThread::handleConnection(int id) {
  handleConnection(id, false);
}

void SiteThread::handleConnection(int id, bool backfromrefresh) {
  if (loggedin > wantedloggedin) {
    connstatetracker[id].setDisconnected();
    conns[id]->doQUITAsync();
    loggedin--;
    return;
  }
  if (!connstatetracker[id].isReady()) {
    return;
  }
  SiteRace * race = NULL;
  if (requests.size() > 0) {
    handleRequest(id);
  }
  else {
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
      conns[id]->doCWDorMKDirAsync(race->getSection(), race->getRelease() + race->getRelevantSubPath());
      if (refresh) {
        connstatetracker[id].setReady();
        if (backfromrefresh) {
          connstatetracker[id].delayedCommand("refresh", SLEEPDELAY, (void *) race);
        }
        else {
          conns[id]->refreshRaceFileListAsync(race);
        }
      }
      else {
        connstatetracker[id].setIdle();
      }
    }
    else {
      connstatetracker[id].setIdle();
      connstatetracker[id].delayedCommand("quit", IDLETIME);
    }
    pthread_mutex_lock(&slots);
    available++;
    pthread_mutex_unlock(&slots);
  }
}

void SiteThread::handleRequest(int id) {
  connstatetracker[id].setReady();
  if (requests.front().requestType() == 0) {
    conns[id]->getFileListAsync(requests.front().requestData(), requests.front().requestId());
  }
  else {
    rawbuf->writeLine(requests.front().requestData());
    conns[id]->getRawAsync(requests.front().requestData(), requests.front().requestId());
  }
  requestsinprogress.push_back(requests.front());
  requests.pop_front();
}

void SiteThread::addRecentList(SiteRace * sr) {
  for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  recentlylistedraces.push_back(sr);
}

bool SiteThread::wasRecentlyListed(SiteRace * sr) {
  for (std::list<SiteRace *>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      return true;
    }
  }
  return false;
}

int SiteThread::requestFileList(std::string path) {
  int requestid = requestidcounter++;
  requests.push_back(SiteThreadRequest(requestid, 0, path));
  sem_post(&notifysem);
  return requestid;
}

int SiteThread::requestRawCommand(std::string command) {
  int requestid = requestidcounter++;
  requests.push_back(SiteThreadRequest(requestid, 1, command));
  sem_post(&notifysem);
  return requestid;
}

bool SiteThread::requestReady(int requestid) {
  std::list<SiteThreadRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return true;
    }
  }
  return false;
}

FileList * SiteThread::getFileList(int requestid) {
  std::list<SiteThreadRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      return (FileList *) it->requestData();
    }
  }
  return NULL;
}

std::string SiteThread::getRawCommandResult(int requestid) {
  std::list<SiteThreadRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      std::string ret = *(std::string *) it->requestData();
      delete (std::string *) it->requestData();
      return ret;
    }
  }
  return "";
}

void SiteThread::finishRequest(int requestid) {
  std::list<SiteThreadRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->requestId() == requestid) {
      requestsready.erase(it);
      return;
    }
  }
  std::list<SiteThreadRequest>::iterator it2;
  for (it2 = requestsinprogress.begin(); it2 != requestsinprogress.end(); it2++) {
    if (it->requestId() == requestid) {
      requestsinprogress.erase(it2);
      return;
    }
  }
}

Site * SiteThread::getSite() {
  return site;
}

SiteRace * SiteThread::getRace(std::string race) {
  for (std::vector<SiteRace *>::iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) return *it;
  }
  return NULL;
}

bool SiteThread::getDownloadThread(FileList * fl, std::string file, FTPThread ** ret) {
  return getReadyThread(fl, file, ret, true, true);
}

bool SiteThread::getUploadThread(FileList * fl, std::string file, FTPThread ** ret) {
  return getReadyThread(fl, file, ret, true, false);
}

bool SiteThread::getReadyThread(FileList * fl, FTPThread ** ret) {
  return getReadyThread(fl, "", ret, false, false);
}

bool SiteThread::getReadyThread(FileList * fl, std::string file, FTPThread ** ret, bool istransfer, bool isdownload) {
  FTPThread * lastready;
  int lastreadyid;
  bool foundreadythread = false;
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isIdle()) {
      foundreadythread = true;
      lastready = conns[i];
      lastreadyid = i;
      if (conns[i]->getCurrentPath().compare(fl->getPath()) == 0) {
        if (istransfer) {
          if (!getSlot(isdownload)) return false;
        }
        connstatetracker[i].setBusy();
        conns[i]->doCWDAsync(fl->getPath());
        *ret = conns[i];
        return true;
      }
    }
  }
  if (!foundreadythread) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isReady()) {
        foundreadythread = true;
        lastready = conns[i];
        lastreadyid = i;
        if (conns[i]->getCurrentPath().compare(fl->getPath()) == 0) {
          if (istransfer) {
            if (!getSlot(isdownload)) return false;
          }
          connstatetracker[i].setBusy();
          conns[i]->doCWDAsync(fl->getPath());
          *ret = conns[i];
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
    lastready->doCWDAsync(fl->getPath());
    *ret = lastready;
    return true;
  }
  else return false;
}

void SiteThread::returnThread(FTPThread * ftpthread) {
  int id = ftpthread->getId();
  connstatetracker[id].setIdle();
  ftpthreadcom->putCommand(id, 8);
}

void SiteThread::setNumConnections(unsigned int num) {
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
        conns[i]->doQUITT();
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
    conns.push_back(new FTPThread(conns.size(), site, ftpthreadcom));
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

bool SiteThread::downloadSlotAvailable() {
  return (available > 0 && slots_dn > 0);
}

bool SiteThread::uploadSlotAvailable() {
  return (available > 0 && slots_up > 0);
}

void SiteThread::transferComplete(bool isdownload) {
  pthread_mutex_lock(&slots);
  if (isdownload) {
    slots_dn++;
  }
  else slots_up++;
  available++;
  pthread_mutex_unlock(&slots);
}

bool SiteThread::getSlot(bool isdownload) {
  pthread_mutex_lock(&slots);
  if (isdownload) {
    if (slots_dn <= 0) {
      pthread_mutex_unlock(&slots);
      return false;
    }
    slots_dn--;
  } else {
    if (slots_up <= 0) {
      pthread_mutex_unlock(&slots);
      return false;
    }
    slots_up--;
  }
  available--;
  pthread_mutex_unlock(&slots);
  return true;
}

void SiteThread::pushPotential(int score, std::string file, SiteThread * dst) {
  int threads = getSite()->getMaxDown();
  ptrack->getFront()->update(dst, threads, dst->getSite()->getMaxDown(), score, file);
}

bool SiteThread::potentialCheck(int score) {
  int max = ptrack->getMaxAvailablePotential();
  if (score > max/2) {
    return true;
  }
  return false;
}

int SiteThread::getCurrLogins() {
  return loggedin;
}

int SiteThread::getCurrDown() {
  return site->getMaxDown() - slots_dn;
}

int SiteThread::getCurrUp() {
  return site->getMaxUp() - slots_up;
}

void SiteThread::connectThread(int id) {
  if (connstatetracker[id].isDisconnected()) {
    if (wantedloggedin < site->getMaxLogins()) {
      wantedloggedin++;
    }
    connstatetracker[id].setIdle();
    conns[id]->loginAsync();
  }
}

void SiteThread::disconnectThread(int id) {
  if (!connstatetracker[id].isDisconnected()) {
    if (wantedloggedin > 0) {
      wantedloggedin--;
    }
    connstatetracker[id].setDisconnected();
    conns[id]->doQUITAsync();
    loggedin--;
  }
}

void SiteThread::issueRawCommand(unsigned int id, std::string command) {
  connectThread(id);
  rawbuf->writeLine(command);
  conns[id]->doRaw(command);
}

RawBuffer * SiteThread::getRawCommandBuffer() {
  return rawbuf;
}

void SiteThread::raceGlobalComplete() {
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
        conns[i]->doQUITAsync();
        loggedin--;
      }
    }
    wantedloggedin = 0;
  }
}

void SiteThread::raceLocalComplete(SiteRace * sr) {
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
        conns[i]->doQUITAsync();
        loggedin--;
        killnum--;
      }
    }
    wantedloggedin = site->getMaxDown();
  }
}

std::vector<FTPThread *> * SiteThread::getConns() {
  return &conns;
}

FTPThread * SiteThread::getConn(int id) {
  std::vector<FTPThread *>:: iterator it;
  for (it = conns.begin(); it != conns.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return NULL;
}

std::string SiteThread::getStatus(int id) {
  if (connstatetracker[id].isIdle()) {
    int idletime = connstatetracker[id].getTimePassed()/1000;
    return "IDLE " + global->int2Str(idletime) + "s";
  }
  return conns[id]->getStatus();
}
