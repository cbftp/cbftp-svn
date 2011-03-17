#include "engine.h"

Engine::Engine() {
  scoreboard = new ScoreBoard();
  race = false;
  maxavgspeed = 1024;
  list_refresh = global->getListRefreshSem();
  sem_init(&race_sem, 0, 0);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
}

void Engine::newRace(std::string release, std::string section, std::list<std::string> sites) {
  Race * race = new Race(release, section);
  for (std::list<std::string>::iterator it = sites.begin(); it != sites.end(); it++) {
    SiteThread * st = global->getSiteThreadManager()->getSiteThread(*it);
    st->addRace(section, release);
    race->addSite(st);
  }
  races.push_back(race);
  setSpeedScale();
  this->race = true;
  sem_post(&race_sem);
}

void * Engine::run(void * arg) {
  ((Engine *) arg)->runInstance();
}

void Engine::runInstance() {
  int runs = 0;
  while(true) {
    if (race) {
      sem_wait(list_refresh);
      refreshScoreBoard();
      std::vector<ScoreBoardElement *> possibles = scoreboard->getElementVector();
      std::cout << "Possible transfers (run " << runs++ << "): " << scoreboard->size() << std::endl;
      for (int i = 0; i < possibles.size(); i++) {
        std::cout << possibles[i]->fileName() << " - " << possibles[i]->getScore() << " - " << possibles[i]->getSource()->getSite()->getName() << " -> " << possibles[i]->getDestination()->getSite()->getName() << std::endl;
      }
      issueOptimalTransfers();
    }
    else sem_wait(&race_sem);
  }
}

void Engine::refreshScoreBoard() {
  scoreboard->wipe();
  for (std::list<Race *>::iterator itr = races.begin(); itr != races.end(); itr++) {
    for (std::list<SiteThread *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      if (!(*its)->downloadSlotAvailable()) continue;
      SiteRace * srs = (*its)->getRace((*itr)->getName());
      FileList * fls = srs->getFileList();
      if (!fls->isFilled()) continue;
      for (std::list<SiteThread *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        if (*itd == *its) continue;
        if (!(*itd)->uploadSlotAvailable()) continue;
        SiteRace * srd = (*itd)->getRace((*itr)->getName());
        FileList * fld = srd->getFileList();
        int avgspeed = (*its)->getSite()->getAverageSpeed((*itd)->getSite()->getName());
        if (!fld->isFilled()) continue;
        std::map<std::string, File *>::iterator itf;
        fls->lockFileList();
        for (itf = fls->begin(); itf != fls->end(); itf++) {
          File * f = (*itf).second;
          std::string name = f->getName();
          if (fld->contains(name) || f->isDirectory() || f->getSize() == 0) continue;
          scoreboard->add(name, calculateScore(f, *its, srs, *itd, srd, avgspeed), *its, srs, *itd, srd);
        }
        fls->unlockFileList();
      }
    }
  }
  scoreboard->sort();
}

void Engine::issueOptimalTransfers() {
  std::vector<ScoreBoardElement *>::iterator it;
  for (it = scoreboard->begin(); it != scoreboard->end(); it++) {
    global->getTransferManager()->newTransfer(*it);
  }
}

int Engine::calculateScore(File * f, SiteThread * sts, SiteRace * srs, SiteThread * std, SiteRace * srd, int avgspeed) {
  int points = 0;
  points += f->getSize() / (srs->getFileList()->getMaxFileSize() / 2000);
  points = (points * (avgspeed / 100)) / (maxavgspeed / 100);
  // give points for owning a low percentage of the race on the target
  points += ((100 - srd->getFileList()->getOwnedPercentage()) * 30);
  // sfv and nfo files have top priority
  if (f->getExtension().compare("sfv") == 0) return 10000;
  else if (f->getExtension().compare("nfo") == 0) return 10000;
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<Race *>::iterator itr = races.begin(); itr != races.end(); itr++) {
    for (std::list<SiteThread *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::list<SiteThread *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        int avgspeed = (*its)->getSite()->getAverageSpeed((*itd)->getSite()->getName());
        if (avgspeed > maxavgspeed) maxavgspeed = avgspeed;
      }
    }
  }
}

