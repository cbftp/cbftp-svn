#include "sitethread.h"

SiteThread::SiteThread(std::string sitename) {
  pthread_mutex_init(&slots, NULL);
  sem_init(&notifysem, 0, 0);
  ftpthreadcom = new FTPThreadCom(&notifysem);
  site = global->getSiteManager()->getSite(sitename);
  slots_dn = site->getMaxDown();
  slots_up = site->getMaxUp();
  available = 0;
  int logins = site->getMaxLogins();
  for (int i = 0; i < logins; i++) {
    conns.push_back(new FTPThread(i, site, ftpthreadcom));
  }
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
}

void SiteThread::activate() {
  for (int i = 0; i < conns.size(); i++) conns[i]->loginAsync();
}

void SiteThread::addRace(std::string section, std::string release) {
  SiteRace * race = new SiteRace(site->getSectionPath(section), release, site->getUser());
  races.push_back(race);
  activate();
}

void * run(void * arg) {
  ((SiteThread *) arg)->runInstance();
}

void SiteThread::runInstance() {
  while(1) {
    sem_wait(&notifysem);
    CommandQueueElement * command = ftpthreadcom->getCommand();
    SiteRace * race = races[0];
    int id = *(int *)command->getArg1();
    int status = *(int *)command->getArg2();
    std::string reply;
    delete (int *)command->getArg1();
    delete (int *)command->getArg2();
    switch(command->getOpcode()) {
      case 0: // login successful
        conns[id]->doCWDorMKDirAsync(race->getSection(), race->getRelease());
        conns[id]->refreshLoopAsync(race);
        conns[id]->setReady();
        pthread_mutex_lock(&slots);
        available++;
        pthread_mutex_unlock(&slots);
        break;
      case 1: // connection failed
        std::cout << "CONNECTION FAILED" << std::endl;
        break;
      case 2: // unknown connect response
        conns[id]->disconnectAsync();
        break;
      case 3: // login user denied
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
        conns[id]->reconnectAsync();
        break;
      case 7: // login kill failed
        delete (char *)command->getArg3();
        conns[id]->doQUITAsync();
        break;
    }
    ftpthreadcom->commandProcessed();
  }
}

Site * SiteThread::getSite() {
  return site;
}

SiteRace * SiteThread::getRace(std::string race) {
  for (std::vector<SiteRace *>::iterator it = races.begin(); it != races.end(); it++) {
    if ((*it)->getRelease().compare(race) == 0) return *it;
  }
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
