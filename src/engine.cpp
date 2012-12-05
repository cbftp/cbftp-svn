#include "engine.h"

Engine::Engine() {
  scoreboard = new ScoreBoard();
  maxavgspeed = 1024;
  list_refresh = global->getListRefreshSem();
  sem_init(&race_sem, 0, 0);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
}

void Engine::newRace(std::string release, std::string section, std::list<std::string> sites) {
  Race * race = new Race(release, section);
  for (std::list<std::string>::iterator it = sites.begin(); it != sites.end(); it++) {
    SiteThread * st = global->getSiteThreadManager()->getSiteThread(*it);
    st->addRace(race, section, release);
    race->addSite(st);
  }
  currentraces.push_back(race);
  allraces.push_back(race);
  setSpeedScale();
  sem_post(&race_sem);
}

void * Engine::run(void * arg) {
  ((Engine *) arg)->runInstance();
  return NULL;
}

void Engine::runInstance() {
  //int runs = 0;
  while(true) {
    if (currentraces.size() > 0) {
      sem_wait(list_refresh);
      estimateRaceSizes();
      refreshScoreBoard();
      std::vector<ScoreBoardElement *> possibles = scoreboard->getElementVector();
      //std::cout << "Possible transfers (run " << runs++ << "): " << scoreboard->size() << std::endl;
      for (unsigned int i = 0; i < possibles.size(); i++) {
        //std::cout << possibles[i]->fileName() << " - " << possibles[i]->getScore() << " - " << possibles[i]->getSource()->getSite()->getName() << " -> " << possibles[i]->getDestination()->getSite()->getName() << std::endl;
      }
      issueOptimalTransfers();
    }
    else sem_wait(&race_sem);
  }
}

void Engine::estimateRaceSizes() {
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    if ((*itr)->sizeEstimated()) {
      continue;
    }
    for (std::list<SiteThread *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      SiteRace * srs = (*its)->getRace((*itr)->getName());
      if (srs->sizeEstimated()) {
        continue;
      }
      FileList * fls = srs->getFileList();
      if (!fls->hasSFV()) {
        continue;
      }

      std::list<std::string> uniques;
      std::map<std::string, File *>::iterator itf;
      fls->lockFileList();
      for (itf = fls->begin(); itf != fls->end(); itf++) {
        if (itf->second->isDirectory()) {
          continue;
        }
        std::string filename = itf->second->getName();
        size_t lastdotpos = filename.rfind(".");
        if (lastdotpos != std::string::npos && lastdotpos < filename.length() - 4) {
          filename = filename.substr(0, lastdotpos + 4);
        }
        std::list<std::string>::iterator it;
        bool exists = false;
        for (it = uniques.begin(); it != uniques.end(); it++) {
          if ((*it) == filename) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          uniques.push_back(filename);
        }
      }
      fls->unlockFileList();
      srs->reportSize(uniques.size());
    }
  }
}

void Engine::refreshScoreBoard() {
  scoreboard->wipe();
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<SiteThread *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      SiteRace * srs = (*its)->getRace((*itr)->getName());
      FileList * fls = srs->getFileList();
      if (!fls->isFilled()) continue;
      if ((*itr)->sizeEstimated() && fls->getNumUploadedFiles() == (*itr)->estimatedSize()) {
        (*its)->raceLocalComplete(srs);
        if ((*itr)->isDone()) {
          for (std::list<SiteThread *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
            (*itd)->raceGlobalComplete();
          }
          currentraces.erase(itr);
          refreshScoreBoard();
          return;
        }
      }
      for (std::list<SiteThread *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        if (*itd == *its) continue;
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
          scoreboard->add(name, calculateScore(f, *itr, *its, srs, *itd, srd, avgspeed, (SPREAD ? false : true)), *its, srs, *itd, srd);
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
    //potentiality handling
    (*it)->getSource()->pushPotential((*it)->getScore(), (*it)->fileName(), (*it)->getDestination());
    if (!(*it)->getSource()->downloadSlotAvailable()) continue;
    if (!(*it)->getDestination()->uploadSlotAvailable()) continue;
    if ((*it)->getSource()->potentialCheck((*it)->getScore())) {
      global->getTransferManager()->newTransfer(*it);
    }
  }
}

int Engine::calculateScore(File * f, Race * itr, SiteThread * sts, SiteRace * srs, SiteThread * std, SiteRace * srd, int avgspeed, bool racemode) {
  int points = 0;
  points += f->getSize() / (srs->getFileList()->getMaxFileSize() / 2000); // gives max 2000 points
  points = (points * (avgspeed / 100)) / (maxavgspeed / 100);
  if (racemode) {
    // give points for owning a low percentage of the race on the target
    points += ((100 - srd->getFileList()->getOwnedPercentage()) * 30); // gives max 3000 points
  }
  else {
    // give points for low progress on the target
    points += 3000 - ((srd->getFileList()->getSizeUploaded() * 3000) / itr->getMaxSiteProgress()); // gives max 3000 points
  }
  // sfv and nfo files have top priority
  if (f->getExtension().compare("sfv") == 0) return 10000;
  else if (f->getExtension().compare("nfo") == 0) return 10000;
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<SiteThread *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::list<SiteThread *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        int avgspeed = (*its)->getSite()->getAverageSpeed((*itd)->getSite()->getName());
        if (avgspeed > maxavgspeed) maxavgspeed = avgspeed;
      }
    }
  }
}

int Engine::currentRaces() {
  return currentraces.size();
}

int Engine::allRaces() {
  return allraces.size();
}

Race * Engine::getRace(std::string releasename) {
  std::list<Race *>::iterator it;
  for (it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == releasename) {
      return *it;
    }
  }
  return NULL;
}

std::list<Race *>::iterator Engine::getRacesIteratorBegin() {
  return allraces.begin();
}

std::list<Race *>::iterator Engine::getRacesIteratorEnd() {
  return allraces.end();
}
