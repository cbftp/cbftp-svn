#include "sitethread.h"

SiteThread::SiteThread(std::string sitename) {
  pthread_mutex_init(&slots, NULL);
  sem_init(&notifysem, 0, 0);
  requestidcounter = 0;
  ftpthreadcom = new FTPThreadCom(&notifysem);
  site = global->getSiteManager()->getSite(sitename);
  slots_dn = site->getMaxDown() > 0 ? site->getMaxDown() : site->getMaxLogins();
  slots_up = site->getMaxUp() > 0 ? site->getMaxUp() : site->getMaxLogins();
  ptrack = new PotentialTracker(slots_dn);
  available = 0;
  loggedin = 0;
  int logins = site->getMaxLogins();
  list_refresh = global->getListRefreshSem();
  for (int i = 0; i < logins; i++) {
    conns.push_back(new FTPThread(i, site, ftpthreadcom));
  }
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
}

void SiteThread::activate() {
  for (int i = 0; i < conns.size(); i++) conns[i]->loginAsync();
}

void SiteThread::addRace(Race * enginerace, std::string section, std::string release) {
  SiteRace * race = new SiteRace(enginerace, site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activate();
}

void * SiteThread::run(void * arg) {
  ((SiteThread *) arg)->runInstance();
}

void SiteThread::runInstance() {
  while(1) {
    sem_wait(&notifysem);
    CommandQueueElement * command = ftpthreadcom->getCommand();
    SiteRace * race;
    int id = *(int *)command->getArg1();
    int status;
    std::string reply;
    std::list<SiteThreadRequest>::iterator it;
    delete (int *)command->getArg1();
    switch(command->getOpcode()) {
      case 0: // login successful
        pthread_mutex_lock(&slots);
        loggedin++;
        pthread_mutex_unlock(&slots);
        if (requests.size() > 0) {
          conns[id]->getFileListAsync(requests.front().requestPath());
          requestsinprogress.push_back(requests.front());
          requests.pop_front();
        }
        else {
          if (races.size() > 0) {
            race = races[0];
            conns[id]->doCWDorMKDirAsync(race->getSection(), race->getRelease());
            conns[id]->refreshLoopAsync(race);
            conns[id]->setReady();
          }
          pthread_mutex_lock(&slots);
          available++;
          pthread_mutex_unlock(&slots);
        }
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
            conns[id]->sleepTickAsync();
            conns[id]->doUSERPASSAsync();
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
            conns[id]->sleepTickAsync();
            conns[id]->reconnectAsync();
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
      case 10: // file list refreshed
        //SiteRace * sr = (SiteRace *)command->getArg3();
        race = races[0];
        race->updateNumFilesUploaded();
        int tmpi;
        sem_getvalue(list_refresh, &tmpi);
        if (tmpi == 0) sem_post(list_refresh);
        break;
      case 11: // file list retrieved
        FileList * filelist = (FileList *)command->getArg2();
        for (it = requestsinprogress.begin(); it != requestsinprogress.end(); it++) {
          if (it->requestPath() == filelist->getPath()) {
            requestsready.push_back(SiteThreadRequestReady(it->requestId(), filelist));
            requestsinprogress.erase(it);
            global->getUICommunicator()->backendPush();
            break;
          }
        }
        break;

    }
    ftpthreadcom->commandProcessed();
  }
}

int SiteThread::requestFileList(std::string path) {
  int requestid = requestidcounter++;
  requests.push_back(SiteThreadRequest(requestid, path));
  if (loggedin == 0) {
    activate();
  }
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
      return it->requestFileList();
    }
  }
  return NULL;
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

bool SiteThread::getDownloadThread(SiteRace * sr, std::string file, FTPThread ** ret) {
  return getReadyThread(sr, file, ret, true, true);
}

bool SiteThread::getUploadThread(SiteRace * sr, std::string file, FTPThread ** ret) {
  return getReadyThread(sr, file, ret, true, false);
}

bool SiteThread::getReadyThread(SiteRace * sr, FTPThread ** ret) {
  return getReadyThread(sr, "", ret, false, false);
}

bool SiteThread::getReadyThread(SiteRace * sr, std::string file, FTPThread ** ret, bool istransfer, bool isdownload) {
  FTPThread * lastready;
  bool foundreadythread = false;
  for (int i = 0; i < conns.size(); i++) {
    if(conns[i]->isReady()) {
      foundreadythread = true;
      lastready = conns[i];
      if (conns[i]->getCurrentPath().compare(sr->getPath()) == 0) {
        *ret = conns[i];
        if (istransfer) {
          if (!getSlot(isdownload)) return false;
        }
        conns[i]->setBusy();
        return true;
      }
    }
  }
  if (foundreadythread) {
    lastready->doCWDAsync(sr->getPath());
    *ret = lastready;
    if (istransfer) {
      if (!getSlot(isdownload)) return false;
    }
    lastready->setBusy();
    return true;
  }
  else return false;
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
