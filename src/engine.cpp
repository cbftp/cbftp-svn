#include "engine.h"

#include <stdlib.h>

#include "scoreboard.h"
#include "scoreboardelement.h"
#include "globalcontext.h"
#include "sitelogic.h"
#include "site.h"
#include "filelist.h"
#include "file.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "race.h"
#include "siterace.h"
#include "skiplist.h"
#include "eventlog.h"
#include "tickpoke.h"
#include "sitemanager.h"
#include "workmanager.h"

Engine::Engine() {
  scoreboard = new ScoreBoard();
  maxavgspeed = 1024;
  pokeregistered = false;
}

void Engine::newRace(std::string release, std::string section, std::list<std::string> sites) {
  Race * race = NULL;
  bool append = false;
  for (std::list<Race *>::iterator it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == release && (*it)->getSection() == section) {
      race = *it;
      append = true;
      break;
    }
  }
  if (race == NULL) {
    race = new Race(release, section);
  }
  std::list<SiteLogic *> addsites;
  for (std::list<std::string>::iterator it = sites.begin(); it != sites.end(); it++) {
    SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (sl == NULL) {
      global->getEventLog()->log("Engine", "Trying to race a nonexisting site: " + *it);
      continue;
    }
    if (!sl->getSite()->hasSection(section)) {
      global->getEventLog()->log("Engine", "Trying to use an undefined section: " +
          section + " on " + *it);
      continue;
    }
    bool add = true;
    for (std::list<SiteLogic *>::iterator it2 = addsites.begin(); it2 != addsites.end(); it2++) {
      if (sl == *it2) {
        add = false;
        break;
      }
    }
    if (add && append) {
      for (std::list<SiteLogic *>::iterator it2 = race->begin(); it2 != race->end(); it2++) {
        if (sl == *it2) {
          add = false;
          break;
        }
      }
    }
    if (add) {
      addsites.push_back(sl);
    }
  }
  if (addsites.size() < 2 && !append) {
    global->getEventLog()->log("Engine", "Ignoring attempt to race " + release + " in "
        + section + " on less than 2 sites.");
    return;
  }
  bool readdtocurrent = true;
  if (addsites.size() > 0) {
    if (!pokeregistered) {
      global->getTickPoke()->startPoke(this, "Engine", POKEINTERVAL, 0);
      pokeregistered = true;
    }
    if (append) {
      for (std::list<Race *>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
        if (*it == race) {
          readdtocurrent = false;
          break;
        }
      }
      if (readdtocurrent) {
        global->getEventLog()->log("Engine", "Reactivating race: " + section + "/" + release);
        race->setUndone();
        for (std::list<SiteLogic *>::iterator it = race->begin(); it != race->end(); it++) {
          (*it)->activate();
        }
      }
    }
    for (std::list<SiteLogic *>::iterator it = addsites.begin(); it != addsites.end(); it++) {
      (*it)->addRace(race, section, release);
      race->addSite((*it));
    }
    if (!append) {
      currentraces.push_back(race);
      allraces.push_back(race);
      dropped = 0;
      global->getEventLog()->log("Engine", "Starting race: " + section + "/" + release +
          " on " + global->int2Str((int)addsites.size()) + " sites.");
    }
    else {
      if (readdtocurrent) {
        currentraces.push_back(race);
      }
      global->getEventLog()->log("Engine", "Appending to race: " + section + "/" + release +
          " with " + global->int2Str((int)addsites.size()) + " sites.");
    }
    setSpeedScale();
  }
}

void Engine::removeSiteFromRace(std::string release, std::string site) {
  for (std::list<Race *>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName() == release) {
      for (std::list<SiteLogic *>::iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++) {
        if ((*it2)->getSite()->getName() == site) {
          (*it2)->abortRace(release);
          (*it)->removeSite(*it2);
          break;
        }
      }
    }
  }
}

void Engine::abortRace(std::string release) {
  for (std::list<Race *>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName() == release) {
      (*it)->abort();
      for (std::list<SiteLogic *>::iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++) {
        (*it2)->abortRace(release);
      }
      currentraces.erase(it);
      break;
    }
  }
}

void Engine::someRaceFileListRefreshed() {
  if (currentraces.size() > 0) {
    if (!global->getWorkManager()->overload()) {
      estimateRaceSizes();
      refreshScoreBoard();
      /*std::vector<ScoreBoardElement *> possibles = scoreboard->getElementVector();
      std::cout << "Possible transfers (run " << runs++ << "): " << scoreboard->size() << std::endl;
      for (unsigned int i = 0; i < possibles.size(); i++) {
        std::cout << possibles[i]->fileName() << " - " << possibles[i]->getScore() << " - " << possibles[i]->getSource()->getSite()->getName() << " -> " << possibles[i]->getDestination()->getSite()->getName() << std::endl;
      }*/
      issueOptimalTransfers();
    }
    else {
      ++dropped;
    }
  }
}

void Engine::estimateRaceSizes() {
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<SiteLogic *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      SiteRace * srs = (*its)->getRace((*itr)->getName());
      std::map<std::string, FileList *>::iterator itfl;
      for (itfl = srs->fileListsBegin(); itfl != srs->fileListsEnd(); itfl++) {
        FileList * fls = itfl->second;
        if (srs->sizeEstimated(fls)) {
          continue;
        }
        reportCurrentSize(srs, fls, false);
        if (fls->hasSFV()) {
          if (srs->getSFVObservedTime(fls) > 5000) {
            reportCurrentSize(srs, fls, true);
          }
        }
        else {
          if (srs->getObservedTime(fls) > 30000) {
            reportCurrentSize(srs, fls, true);
          }
        }
      }
    }
  }
}

void Engine::reportCurrentSize(SiteRace * srs, FileList * fls, bool final) {
  std::list<std::string> uniques;
  std::map<std::string, File *>::iterator itf;
  std::string subpath = srs->getSubPathForFileList(fls);
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
    if (!global->getSkipList()->isAllowed(filename) ||
        (filename != "" && !global->getSkipList()->isAllowed(subpath + "/" + filename))) {
      continue;
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
  srs->reportSize(fls, &uniques, final);
}

void Engine::refreshScoreBoard() {
  scoreboard->wipe();
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    Race * race = *itr;
    for (std::list<SiteLogic *>::iterator its = race->begin(); its != race->end(); its++) {
      SiteLogic * sls = *its;
      SiteRace * srs = sls->getRace(race->getName());
      if (!srs->isDone()) {
        bool racecomplete = true;
        std::list<std::string> subpaths = race->getSubPaths();
        for (std::list<std::string>::iterator itsp = subpaths.begin(); itsp != subpaths.end(); itsp++) {
          FileList * spfl = srs->getFileListForPath(*itsp);
          if (spfl != NULL && race->sizeEstimated(*itsp) &&
              spfl->getNumUploadedFiles() >= race->estimatedSize(*itsp)) {
            if (!srs->isSubPathComplete(spfl)) {
              srs->subPathComplete(spfl);
            }
          }
          else {
            racecomplete = false;
          }
        }
        if (racecomplete) {
          sls->raceLocalComplete(srs);
          global->getEventLog()->log("Engine", "Race " + race->getName() + " completed on " +
              sls->getSite()->getName());
          if (race->isDone()) {
            issueGlobalComplete(race);
            currentraces.erase(itr);
            refreshScoreBoard();
            global->getEventLog()->log("Engine", "Race globally completed: " + race->getName());
            if (dropped) {
              global->getEventLog()->log("Engine", "Scoreboard refreshes dropped since race start: " + global->int2Str(dropped));
            }
            return;
          }
        }
      }
      for (std::list<SiteLogic *>::iterator itd = race->begin(); itd != race->end(); itd++) {
        SiteLogic * sld = *itd;
        if (sls == sld) continue;
        if (global->getSiteManager()->isBlockedPair(sls->getSite(), sld->getSite())) continue;
        if (sld->getSite()->isAffiliated(race->getGroup())) continue;
        SiteRace * srd = sld->getRace(race->getName());
        int avgspeed = sls->getSite()->getAverageSpeed(sld->getSite()->getName());
        if (avgspeed > maxavgspeed) {
          avgspeed = maxavgspeed;
        }
        for (std::map<std::string, FileList *>::iterator itfls = srs->fileListsBegin(); itfls != srs->fileListsEnd(); itfls++) {
          if (!global->getSkipList()->isAllowed(itfls->first)) continue;
          FileList * fls = itfls->second;
          FileList * fld = srd->getFileListForPath(itfls->first);
          if (fld != NULL) {
            if (!fld->isFilled()) continue;
            std::map<std::string, File *>::iterator itf;
            fls->lockFileList();
            for (itf = fls->begin(); itf != fls->end(); itf++) {
              File * f = itf->second;
              if (!global->getSkipList()->isAllowed(itf->first) ||
                  (itfls->first != "" && !global->getSkipList()->isAllowed(itfls->first + "/" + itf->first))) {
                continue;
              }
              std::string filename = f->getName();
              if (fld->contains(filename) || f->isDirectory() || f->getSize() == 0) continue;
              if (fls->hasFailedDownload(filename)) continue;
              if (fld->hasFailedUpload(filename)) continue;
              if (!sls->getSite()->getAllowDownload()) continue;
              if (!sld->getSite()->getAllowUpload()) continue;
              if (sls->getSite()->hasBrokenPASV() &&
                  sld->getSite()->hasBrokenPASV()) continue;
              //ssl check
              if ((sls->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_OFF &&
                  sld->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_ON) ||
                  (sls->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_ON &&
                      sld->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_OFF)) {
                continue;
              }
              bool prio = false;
              int score = calculateScore(f, race, fls, srs, fld, srd, avgspeed, &prio, (SPREAD ? false : true));
              scoreboard->add(filename, score, prio, sls, fls, sld, fld);
              race->resetUpdateCheckCounter();
            }
            fls->unlockFileList();
          }
          else {
            srd->addSubDirectory(itfls->first);
          }
        }
      }
    }
  }
  scoreboard->sort();
}

void Engine::issueOptimalTransfers() {
  std::vector<ScoreBoardElement *>::iterator it;
  ScoreBoardElement * sbe;
  SiteLogic * sls;
  SiteLogic * sld;
  std::string filename;
  for (it = scoreboard->begin(); it != scoreboard->end(); it++) {
    sbe = *it;
    sls = sbe->getSource();
    sld = sbe->getDestination();
    filename = sbe->fileName();
    //potentiality handling
    if (!sbe->isPrioritized()) { // priority files shouldn't affect the potential tracking
      sls->pushPotential(sbe->getScore(), filename, sld);
    }
    if (!sls->downloadSlotAvailable()) continue;
    if (!sld->uploadSlotAvailable()) continue;
    if (sls->potentialCheck(sbe->getScore())) {
      global->getTransferManager()->suggestTransfer(filename, sls,
          sbe->getSourceFileList(), sld, sbe->getDestinationFileList());
    }
  }
}

int Engine::calculateScore(File * f, Race * itr, FileList * fls, SiteRace * srs, FileList * fld, SiteRace * srd, int avgspeed, bool * prio, bool racemode) {
  int points = 0;
  unsigned long long int filesize = f->getSize();
  unsigned long long int maxfilesize = srs->getMaxFileSize();
  if (filesize > maxfilesize) {
    maxfilesize = filesize;
  }
  points += filesize / ((maxfilesize + 2000) / 2000); // gives max 2000 points
  points = (points * (avgspeed / 100)) / (maxavgspeed / 100);
  if (racemode) {
    // give points for owning a low percentage of the race on the target
    points += ((100 - fld->getOwnedPercentage()) * 30); // gives max 3000 points
  }
  else {
    // give points for low progress on the target
    int maxprogress = itr->getMaxSiteProgress();
    if (maxprogress > 0) {
      points += 3000 - ((fld->getSizeUploaded() * 3000) / maxprogress); // gives max 3000 points
    }
  }
  // sfv and nfo files have top priority
  if (f->getExtension().compare("sfv") == 0 ||
      f->getExtension().compare("nfo") == 0) {
    *prio = true;
    return 10000;
  }
  if (points > 10000 || points < 0) {
    global->getEventLog()->log("Engine", "BUG: unexpected score. Avgspeed: " +
        global->int2Str(avgspeed) + " Maxavgspeed: " + global->int2Str(maxavgspeed) +
        " Filesize: " + global->int2Str(f->getSize()) + " Maxfilesize: " +
        global->int2Str(srs->getMaxFileSize()) + " Ownedpercentage: " +
        global->int2Str(fld->getOwnedPercentage()) + " Maxprogress: " +
        global->int2Str(itr->getMaxSiteProgress()));
  }
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<Race *>::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<SiteLogic *>::iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::list<SiteLogic *>::iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
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

void Engine::tick(int message) {
  for (std::list<Race *>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->checksSinceLastUpdate() >= MAXCHECKSTIMEOUT) {
      global->getEventLog()->log("Engine", "No activity for " + global->int2Str(MAXCHECKSTIMEOUT) +
          " seconds, aborting race: " + (*it)->getName());
      for (std::list<SiteLogic *>::iterator its = (*it)->begin(); its != (*it)->end(); its++) {
        (*its)->raceLocalComplete((*its)->getRace((*it)->getName()));
      }
      issueGlobalComplete(*it);
      currentraces.erase(it);
      break;
    }
  }
}

void Engine::issueGlobalComplete(Race * race) {
  for (std::list<SiteLogic *>::iterator itd = race->begin(); itd != race->end(); itd++) {
    (*itd)->raceGlobalComplete();
  }
}

ScoreBoard * Engine::getScoreBoard() {
  return scoreboard;
}
