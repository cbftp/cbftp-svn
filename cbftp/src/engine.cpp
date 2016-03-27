#include "engine.h"

#include <stdlib.h>

#include "core/workmanager.h"
#include "core/tickpoke.h"
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
#include "sitemanager.h"
#include "transferjob.h"
#include "pendingtransfer.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "transferstatus.h"
#include "util.h"
#include "preparedrace.h"

extern GlobalContext * global;

int getPriorityPoints(int priority) {
  switch (priority) {
    case SITE_PRIORITY_VERY_LOW:
      return 0;
    case SITE_PRIORITY_LOW:
      return 500;
    case SITE_PRIORITY_NORMAL:
      return 1000;
    case SITE_PRIORITY_HIGH:
      return 1500;
    case SITE_PRIORITY_VERY_HIGH:
      return 2000;
  }
  return 1000;
}

Engine::Engine() :
  scoreboard(makePointer<ScoreBoard>()),
  maxavgspeed(1024),
  pokeregistered(false),
  nextid(0) {
}

Engine::~Engine() {

}

Pointer<Race> Engine::newSpreadJob(int profile, const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  Pointer<Race> race;
  bool append = false;
  for (std::list<Pointer<Race> >::iterator it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == release && (*it)->getSection() == section) {
      race = *it;
      append = true;
      break;
    }
  }
  if (!global->getSkipList()->isAllowed(release, true, false)) {
    global->getEventLog()->log("Engine", "Race skipped due to skiplist match: " + release);
    return Pointer<Race>();
  }
  if (release.find("/") != std::string::npos) {
    global->getEventLog()->log("Engine", "Race skipped due to invalid target: " + release);
    return Pointer<Race>();
  }
  if (!race) {
    race = makePointer<Race>(nextid++, static_cast<SpreadProfile>(profile), release, section);
  }
  std::list<std::string> addsites;
  std::list<SiteLogic *> addsiteslogics;
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
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
    if (checkBannedGroup(sl->getSite(), race->getGroup())) {
      continue;
    }
    bool add = true;
    for (std::list<std::string>::iterator it2 = addsites.begin(); it2 != addsites.end(); it2++) {
      if (*it == *it2) {
        add = false;
        break;
      }
    }
    if (add && append) {
      if (race->getSiteRace(*it) != NULL) {
        continue;
      }
    }
    if (add) {
      addsites.push_back(*it);
      addsiteslogics.push_back(sl);
    }
  }
  if (addsites.size() < 2 && !append) {
    global->getEventLog()->log("Engine", "Ignoring attempt to race " + release + " in "
        + section + " on less than 2 sites.");
    return Pointer<Race>();
  }
  bool readdtocurrent = true;
  if (addsites.size() > 0 || append) {
    checkStartPoke();
    if (profile == SPREAD_PREPARE) {
      global->getEventLog()->log("Engine", "Preparing race: " + section + "/" + release +
                " on " + util::int2Str((int)addsites.size()) + " sites.");
      preparedraces.push_back(makePointer<PreparedRace>(race->getId(), release, section, addsites));
      for (std::list<SiteLogic *>::const_iterator ait = addsiteslogics.begin(); ait != addsiteslogics.end(); ait++) {
        (*ait)->activateAll();
      }
      return Pointer<Race>();
    }
    if (append) {
      for (std::list<Pointer<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
        if (*it == race) {
          readdtocurrent = false;
          break;
        }
      }
      if (readdtocurrent) {
        global->getEventLog()->log("Engine", "Reactivating race: " + section + "/" + release);
        race->setUndone();
        for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = race->begin(); it != race->end(); it++) {
          it->second->activateAll();
        }
      }
    }
    for (std::list<std::string>::iterator it = addsites.begin(); it != addsites.end(); it++) {
      addSiteToRace(race, *it);
    }
    if (!append) {
      currentraces.push_back(race);
      allraces.push_back(race);
      dropped = 0;
      global->getEventLog()->log("Engine", "Starting race: " + section + "/" + release +
          " on " + util::int2Str((int)addsites.size()) + " sites.");
    }
    else if (addsites.size()) {
      if (readdtocurrent) {
        currentraces.push_back(race);
      }
      global->getEventLog()->log("Engine", "Appending to race: " + section + "/" + release +
          " with " + util::int2Str((int)addsites.size()) + " site" + (addsites.size() > 1 ? "s" : "") + ".");
    }
    setSpeedScale();
  }
  return race;
}

Pointer<Race> Engine::newSpreadJob(int profile, const std::string & release, const std::string & section) {
  std::list<std::string> sites;
  for (std::vector<Site *>::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
    if ((*it)->hasSection(section)) {
      sites.push_back((*it)->getName());
    }
  }
  return newSpreadJob(profile, release, section, sites);
}

Pointer<Race> Engine::newRace(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  return newSpreadJob(SPREAD_RACE, release, section, sites);
}

Pointer<Race> Engine::newRace(const std::string & release, const std::string & section) {
  return newSpreadJob(SPREAD_RACE, release, section);
}

Pointer<Race> Engine::newDistribute(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  return newSpreadJob(SPREAD_DISTRIBUTE, release, section, sites);
}

void Engine::prepareRace(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  newSpreadJob(SPREAD_PREPARE, release, section, sites);
}

void Engine::prepareRace(const std::string & release, const std::string & section) {
  newSpreadJob(SPREAD_PREPARE, release, section);
}

void Engine::startPreparedRace(unsigned int id) {
  for (std::list<Pointer<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    if ((*it)->getId() == id) {
      newRace((*it)->getRelease(), (*it)->getSection(), (*it)->getSites());
      preparedraces.erase(it);
      return;
    }
  }
}

void Engine::deletePreparedRace(unsigned int id) {
  for (std::list<Pointer<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    if ((*it)->getId() == id) {
      preparedraces.erase(it);
      return;
    }
  }
}

void Engine::startLatestPreparedRace() {
  if (preparedraces.size()) {
    Pointer<PreparedRace> preparedrace = preparedraces.back();
    preparedraces.pop_back();
    newRace(preparedrace->getRelease(), preparedrace->getSection(), preparedrace->getSites());
  }
}

void Engine::newTransferJobDownload(std::string site, std::string file, FileList * filelist, std::string path) {
  newTransferJobDownload(site, file, filelist, path, file);
}

void Engine::newTransferJobUpload(std::string path, std::string site, std::string file, FileList * filelist) {
  newTransferJobUpload(path, file, site, file, filelist);
}

void Engine::newTransferJobFXP(std::string srcsite, FileList * srcfilelist, std::string dstsite, FileList * dstfilelist, std::string file) {
  newTransferJobFXP(srcsite, file, srcfilelist, dstsite, file, dstfilelist);
}

void Engine::newTransferJobDownload(std::string site, std::string srcfile, FileList * filelist, std::string path, std::string dstfile) {
  SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(site);
  Pointer<TransferJob> tj = makePointer<TransferJob>(nextid++, sl, srcfile, filelist, path, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting download job: " + srcfile +
            " from " + site);
  sl->addTransferJob(tj);
  checkStartPoke();
}

void Engine::newTransferJobUpload(std::string path, std::string srcfile, std::string site, std::string dstfile, FileList * filelist) {
  SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(site);
  Pointer<TransferJob> tj = makePointer<TransferJob>(nextid++, path, srcfile, sl, dstfile, filelist);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting upload job: " + srcfile +
            " to " + site);
  sl->addTransferJob(tj);
  checkStartPoke();
}

void Engine::newTransferJobFXP(std::string srcsite, std::string srcfile, FileList * srcfilelist, std::string dstsite, std::string dstfile, FileList * dstfilelist) {
  SiteLogic * slsrc = global->getSiteLogicManager()->getSiteLogic(srcsite);
  SiteLogic * sldst = global->getSiteLogicManager()->getSiteLogic(dstsite);
  Pointer<TransferJob> tj = makePointer<TransferJob>(nextid++, slsrc, srcfile, srcfilelist, sldst, dstfile, dstfilelist);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting FXP job: " + srcfile +
            " - " + srcsite + " -> " + dstsite);
  slsrc->addTransferJob(tj);
  sldst->addTransferJob(tj);
  checkStartPoke();
}

void Engine::removeSiteFromRace(Pointer<Race> & race, const std::string & site) {
  if (!!race) {
    SiteRace * sr = race->getSiteRace(site);
    if (sr != NULL) {
      SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(site);
      sl->abortRace(race->getId());
      race->removeSite(sr);
    }
  }
}

void Engine::abortRace(Pointer<Race> & race) {
  if (!!race) {
    race->abort();
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = race->begin(); it != race->end(); it++) {
      it->second->abortRace(race->getId());
    }
    currentraces.remove(race);
    global->getEventLog()->log("Engine", "Race aborted: " + race->getName());
  }
}

void Engine::resetRace(Pointer<Race> & race) {
  if (!!race) {
    race->reset();
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = race->begin(); it != race->end(); it++) {
      it->first->reset();
      it->second->activateAll();
    }
    bool current = false;
    for (std::list<Pointer<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
      if (*it == race) {
        current = true;
        break;
      }
    }
    if (!current) {
      currentraces.push_back(race);
    }
    checkStartPoke();
    global->getEventLog()->log("Engine", "Race reset: " + race->getName());
  }
}

void Engine::deleteOnAllSites(Pointer<Race> & race) {
  if (!!race) {
    std::string sites;
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = race->begin(); it != race->end(); it++) {
      std::string path = it->first->getPath();
      it->second->requestDelete(path, true, false);
      sites += it->first->getSiteName() + ",";
    }
    if (sites.length() > 0) {
      sites = sites.substr(0, sites.length() - 1);
      global->getEventLog()->log("Engine", "Attempting delete of " + race->getName() + " on: " + sites);
    }
  }
}

void Engine::abortTransferJob(Pointer<TransferJob> & tj) {
  tj->abort();
  currenttransferjobs.remove(tj);
  global->getEventLog()->log("Engine", "Transfer job aborted: " + tj->getSrcFileName());
}

void Engine::raceFileListRefreshed(SiteLogic * sls, SiteRace * sr) {
  Pointer<Race> race = sr->getRace();
  if (currentraces.size() > 0) {
    if (!global->getWorkManager()->overload()) {
      estimateRaceSizes();
      checkIfRaceComplete(sls, race);
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

bool Engine::transferJobActionRequest(Pointer<TransferJob> & tj) {
  std::map<Pointer<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
    it = pendingtransfers.find(tj);
  }
  if (!tj->isInitialized()) {
    tj->setInitialized();
  }
  if (it->second.size() == 0) {
    refreshPendingTransferList(tj);
    if (it->second.size() == 0) {
      tj->refreshOrAlmostDone();
      return true;
    }
  }
  if (tj->listsRefreshed()) {
    tj->clearRefreshLists();
  }
  PendingTransfer pt = it->second.front();
  switch (pt.type()) {
    case PENDINGTRANSFER_DOWNLOAD:
    {
      if (!pt.getSrc()->downloadSlotAvailable()) return false;
      Pointer<TransferStatus> ts = global->getTransferManager()->suggestDownload(pt.getSrcFileName(),
          pt.getSrc(), pt.getSrcFileList(), pt.getLocalFileList());
      tj->addTransfer(ts);
      break;
    }
    case PENDINGTRANSFER_UPLOAD:
    {
      if (!pt.getDst()->uploadSlotAvailable()) return false;
      Pointer<TransferStatus> ts = global->getTransferManager()->suggestUpload(pt.getSrcFileName(),
          pt.getLocalFileList(), pt.getDst(), pt.getDstFileList());
      tj->addTransfer(ts);
      break;
    }
    case PENDINGTRANSFER_FXP:
    {
      if (!pt.getSrc()->downloadSlotAvailable()) return false;
      if (!pt.getDst()->uploadSlotAvailable()) return false;
      if (pt.getDst() == pt.getSrc() && pt.getDst()->slotsAvailable() < 2) {
        pt.getDst()->haveConnected(2);
        return false;
      }
      Pointer<TransferStatus> ts = global->getTransferManager()->suggestTransfer(pt.getSrcFileName(),
          pt.getSrc(), pt.getSrcFileList(), pt.getDst(), pt.getDstFileList());
      tj->addTransfer(ts);
      break;
    }
  }
  it->second.pop_front();
  return true;
}

void Engine::estimateRaceSizes() {
  for (std::list<Pointer<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      SiteRace * srs = its->first;
      std::map<std::string, FileList *>::const_iterator itfl;
      for (itfl = srs->fileListsBegin(); itfl != srs->fileListsEnd(); itfl++) {
        FileList * fls = itfl->second;
        if (srs->sizeEstimated(fls)) {
          continue;
        }
        reportCurrentSize(srs, fls, false);
        if (fls->hasSFV()) {
          if (srs->getSFVObservedTime(fls) > SFVDIROBSERVETIME) {
            reportCurrentSize(srs, fls, true);
          }
        }
        else {
          if (srs->getObservedTime(fls) > DIROBSERVETIME) {
            reportCurrentSize(srs, fls, true);
          }
        }
      }
    }
  }
}

void Engine::reportCurrentSize(SiteRace * srs, FileList * fls, bool final) {
  std::list<std::string> uniques;
  std::map<std::string, File *>::const_iterator itf;
  std::string subpath = srs->getSubPathForFileList(fls);
  for (itf = fls->begin(); itf != fls->end(); itf++) {
    File * file = itf->second;
    bool isdir = file->isDirectory();
    if (isdir) {
      continue;
    }
    std::string filename = file->getName();
    size_t lastdotpos = filename.rfind(".");
    if (lastdotpos != std::string::npos && lastdotpos < filename.length() - 4) {
      int offsetdot = 4;
      if (file->getSize() == 0 && lastdotpos > 0 && lastdotpos == filename.length() - 8 &&
          filename.substr(lastdotpos) == ".missing") { // special hack for some zipscripts
        offsetdot = -1;
      }
      filename = filename.substr(0, lastdotpos + offsetdot);
    }
    std::string prepend = subpath.length() ? subpath + "/" : "";
    if (!global->getSkipList()->isAllowed(prepend + filename, isdir)) {
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
  srs->reportSize(fls, &uniques, final);
}

void Engine::refreshScoreBoard() {
  scoreboard->wipe();
  for (std::list<Pointer<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    Pointer<Race> race = *itr;
    bool racemode = race->getProfile() == SPREAD_RACE;
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator its = race->begin(); its != race->end(); its++) {
      SiteRace * srs = its->first;
      SiteLogic * sls = its->second;
      if (!sls->getSite()->getAllowDownload()) continue;
      for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator itd = race->begin(); itd != race->end(); itd++) {
        SiteRace * srd = itd->first;
        SiteLogic * sld = itd->second;
        if (sls == sld) continue;
        if (!sld->getSite()->getAllowUpload()) continue;
        if (global->getSiteManager()->isBlockedPair(sls->getSite(), sld->getSite())) continue;
        if (sld->getSite()->isAffiliated(race->getGroup())) continue;
        if (sls->getSite()->hasBrokenPASV() &&
            sld->getSite()->hasBrokenPASV()) continue;
        //ssl check
        if ((sls->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_OFF &&
            sld->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_ON) ||
            (sls->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_ON &&
                sld->getSite()->getSSLTransferPolicy() == SITE_SSL_ALWAYS_OFF)) {
          continue;
        }
        if (!global->getSiteManager()->testRankCompatibility(*sls->getSite(), *sld->getSite())) continue;
        int avgspeed = sls->getSite()->getAverageSpeed(sld->getSite()->getName());
        if (avgspeed > maxavgspeed) {
          avgspeed = maxavgspeed;
        }
        int prioritypoints = getPriorityPoints(sld->getSite()->getPriority());
        for (std::map<std::string, FileList *>::const_iterator itfls = srs->fileListsBegin(); itfls != srs->fileListsEnd(); itfls++) {
          if (itfls->first.length() > 0 && !global->getSkipList()->isAllowed(itfls->first, true)) continue;
          FileList * fls = itfls->second;
          FileList * fld = srd->getFileListForPath(itfls->first);
          if (fld != NULL) {
            if (!fld->isFilled()) continue;
            std::map<std::string, File *>::const_iterator itf;
            for (itf = fls->begin(); itf != fls->end(); itf++) {
              File * f = itf->second;
              const bool isdir = f->isDirectory();
              const std::string prepend = itfls->first.length() ? itfls->first + "/" : "";
              if (!global->getSkipList()->isAllowed(prepend + itf->first, isdir)) {
                continue;
              }
              const std::string filename = f->getName();
              if (fld->contains(filename) || f->isDirectory() || f->getSize() == 0) continue;
              if (race->hasFailedTransfer(f, fld)) continue;
              bool prio = false;
              unsigned short score = calculateScore(f, race, fls, srs, fld, srd, avgspeed, &prio, prioritypoints, racemode);
              scoreboard->add(filename, score, prio, sls, fls, sld, fld, race);
              race->resetUpdateCheckCounter();
            }
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

void Engine::refreshPendingTransferList(Pointer<TransferJob> & tj) {
  std::map<Pointer<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
  }
  it = pendingtransfers.find(tj);
  std::list<PendingTransfer> & list = it->second;
  list.clear();
  tj->clearExisting();
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD: {
      std::map<std::string, FileList *>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        FileList * srclist = it2->second;
        Pointer<LocalFileList> dstlist = tj->findLocalFileList(it2->first);
        for (std::map<std::string, File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          if (!srcit->second->isDirectory() && srcit->second->getSize() > 0) {
            std::map<std::string, LocalFile>::const_iterator dstit;
            if (!dstlist) {
              dstlist = tj->wantedLocalDstList(it2->first);
            }
            dstit = dstlist->find(srcit->first);
            std::string subpath = it2->first.length() > 0 ? it2->first + "/" : "";
            if (!dstlist || dstit == dstlist->end() || dstit->second.getSize() == 0) {
              PendingTransfer p(tj->getSrc(), srclist, srcit->first, dstlist, srcit->first);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath + srcit->first, srcit->second->getSize());
            }
            else {
              tj->targetExists(subpath + srcit->first);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_DOWNLOAD_FILE:
      if (tj->getSrcFileList()->getFile(tj->getSrcFileName())->getSize() > 0) {
        Pointer<LocalFileList> dstlist = tj->getLocalFileList();
        std::map<std::string, LocalFile>::const_iterator dstit;
        if (!!dstlist) {
          dstit = dstlist->find(tj->getDstFileName());
        }
        if (!dstlist || dstit == dstlist->end() || dstit->second.getSize() == 0) {
          PendingTransfer p(tj->getSrc(), tj->getSrcFileList(),
              tj->getSrcFileName(), dstlist, tj->getDstFileName());
          addPendingTransfer(list, p);
          tj->addPendingTransfer(tj->getSrcFileName(),
              tj->getSrcFileList()->getFile(tj->getSrcFileName())->getSize());
        }
        else {
          tj->targetExists(tj->getSrcFileName());
        }
      }
      break;
    case TRANSFERJOB_UPLOAD: {
      std::map<std::string, Pointer<LocalFileList> >::const_iterator lit;
      for (lit = tj->localFileListsBegin(); lit != tj->localFileListsEnd(); lit++) {
        FileList * dstlist = tj->findDstList(lit->first);
        for (std::map<std::string, LocalFile>::const_iterator lfit = lit->second->begin(); lfit != lit->second->end(); lfit++) {
          if (!lfit->second.isDirectory() && lfit->second.getSize() > 0) {
            if (dstlist == NULL) {
              tj->wantDstDirectory(lit->first);
              break;
            }
            std::string filename = lfit->first;
            std::string subpath = lit->first.length() > 0 ? lit->first + "/" : "";
            if (dstlist->getFile(filename) == NULL) {
              PendingTransfer p(lit->second, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath + filename, lfit->second.getSize());
            }
            else {
              tj->targetExists(subpath + filename);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_UPLOAD_FILE: {
      Pointer<LocalFileList> srclist = tj->getLocalFileList();
      std::map<std::string, LocalFile>::const_iterator srcit;
      if (!!srclist) {
        srcit = srclist->find(tj->getSrcFileName());
      }
      if (!!srclist && srcit != srclist->end() && srcit->second.getSize() > 0 &&
          tj->getDstFileList()->getFile(tj->getDstFileName()) == NULL)
      {
        PendingTransfer p(srclist, tj->getSrcFileName(), tj->getDst(), tj->getDstFileList(), tj->getDstFileName());
        addPendingTransfer(list, p);
        tj->addPendingTransfer(tj->getSrcFileName(), srcit->second.getSize());
      }
      else {
        tj->targetExists(tj->getSrcFileName());
      }
      break;
    }
    case TRANSFERJOB_FXP: {
      std::map<std::string, FileList *>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        FileList * srclist = it2->second;
        FileList * dstlist = tj->findDstList(it2->first);
        for (std::map<std::string, File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          if (!srcit->second->isDirectory() && srcit->second->getSize() > 0) {

            if (dstlist == NULL) {
              tj->wantDstDirectory(it2->first);
              break;
            }
            std::string filename = srcit->first;
            std::string subpath = it2->first.length() > 0 ? it2->first + "/" : "";
            if (dstlist->getFile(filename) == NULL) {
              PendingTransfer p(tj->getSrc(), srclist, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);

              tj->addPendingTransfer(subpath + filename, srcit->second->getSize());
            }
            else {
              tj->targetExists(subpath + filename);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_FXP_FILE:
      if (tj->getSrcFileList()->getFile(tj->getSrcFileName())->getSize() > 0) {
        if (tj->getDstFileList()->getFile(tj->getDstFileName()) == NULL) {
          PendingTransfer p(tj->getSrc(), tj->getSrcFileList(),
                        tj->getSrcFileName(), tj->getDst(), tj->getDstFileList(), tj->getDstFileName());
          addPendingTransfer(list, p);
          tj->addPendingTransfer(tj->getSrcFileName(),
              tj->getSrcFileList()->getFile(tj->getSrcFileName())->getSize());
        }
        else {
          tj->targetExists(tj->getSrcFileName());
        }
      }
      break;
  }
}

void Engine::addPendingTransfer(std::list<PendingTransfer> & list, PendingTransfer & p) {
  std::string extension = File::getExtension(p.getSrcFileName());
  if (extension == "sfv" || extension == "nfo") {
    list.push_front(p); // sfv and nfo files have top priority
  }
  else {
    list.push_back(p);
  }
}

void Engine::issueOptimalTransfers() {
  std::vector<ScoreBoardElement *>::const_iterator it;
  ScoreBoardElement * sbe;
  SiteLogic * sls;
  SiteLogic * sld;
  std::string filename;
  Pointer<Race> race;
  for (it = scoreboard->begin(); it != scoreboard->end(); it++) {
    sbe = *it;
    sls = sbe->getSource();
    sld = sbe->getDestination();
    race = sbe->getRace();
    filename = sbe->fileName();
    //potentiality handling
    if (!sbe->isPrioritized()) { // priority files shouldn't affect the potential tracking
      sls->pushPotential(sbe->getScore(), filename, sld);
    }
    if (!sls->downloadSlotAvailable()) continue;
    if (!sld->uploadSlotAvailable()) continue;
    if (!sls->potentialCheck(sbe->getScore())) continue;
    Pointer<TransferStatus> ts =
      global->getTransferManager()->suggestTransfer(filename, sls,
        sbe->getSourceFileList(), sld, sbe->getDestinationFileList());
    if (!!ts) {
      race->addTransfer(ts);
    }
  }
}

void Engine::checkIfRaceComplete(SiteLogic * sls, Pointer<Race> & race) {
  SiteRace * srs = sls->getRace(race->getName());
  if (!srs->isDone()) {
    bool unfinisheddirs = false;
    bool emptydirs = false;
    int completedlists = 0;
    std::list<std::string> subpaths = race->getSubPaths();
    for (std::list<std::string>::iterator itsp = subpaths.begin(); itsp != subpaths.end(); itsp++) {
      FileList * spfl = srs->getFileListForPath(*itsp);
      if (spfl != NULL && spfl->isFilled()) {
        if (!race->sizeEstimated(*itsp)) {
          if (spfl->getSize() > 0) {
            unfinisheddirs = true;
            continue;
          }
          emptydirs = true;
        }
        else if (spfl->getNumUploadedFiles() >= race->estimatedSize(*itsp) &&
          spfl->timeSinceLastChanged() > STATICTIMEFORCOMPLETION &&
          sls->getCurrLogins() > sls->getCurrUp() + sls->getCurrDown() &&
          !spfl->hasFilesUploading()) {
          completedlists++;
          if (!srs->isSubPathComplete(spfl)) {
            //global->getEventLog()->log("Engine", "Dir marked as complete: " + spfl->getPath() + " on " +
            //  sls->getSite()->getName());
            srs->subPathComplete(spfl);
          }
        }
        else {
          unfinisheddirs = true;
          continue;
        }
      }
      else {
        unfinisheddirs = true;
        continue;
      }
    }
    if (completedlists > 0 && !unfinisheddirs) {
      if (emptydirs) {
        race->reportSemiDone(srs);
      }
      else {
        int uploadslotcount = 0;
        for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = race->begin(); it != race->end(); it++) {
          if (it->second != sls && !it->first->isDone()) {
            uploadslotcount += it->second->getSite()->getMaxUp();
          }
        }
        sls->raceLocalComplete(srs, uploadslotcount);
        global->getEventLog()->log("Engine", "Race " + race->getName() + " completed on " +
            sls->getSite()->getName());
      }
      if (race->isDone()) {
        raceComplete(race);
      }
    }
  }
}

void Engine::raceComplete(Pointer<Race> race) {
  issueGlobalComplete(race);
  for (std::list<Pointer<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it) == race) {
      currentraces.erase(it);
      break;
    }
  }
  refreshScoreBoard();
  global->getEventLog()->log("Engine", "Race globally completed: " + race->getName());
  if (dropped) {
    global->getEventLog()->log("Engine", "Scoreboard refreshes dropped since race start: " + util::int2Str(dropped));
  }
  return;
}

void Engine::transferJobComplete(Pointer<TransferJob> tj) {

  for (std::list<Pointer<TransferJob> >::iterator it = currenttransferjobs.begin(); it != currenttransferjobs.end(); it++) {
    if ((*it) == tj) {
      currenttransferjobs.erase(it);
      break;
    }
  }
  global->getEventLog()->log("Engine", tj->typeString() + " job complete: " + tj->getSrcFileName());
}

unsigned short Engine::calculateScore(File * f, Pointer<Race> & itr, FileList * fls, SiteRace * srs, FileList * fld, SiteRace * srd, int avgspeed, bool * prio, int prioritypoints, bool racemode) const {
  unsigned short points = 0;
  unsigned long long int filesize = f->getSize();
  unsigned long long int maxfilesize = srs->getMaxFileSize();
  if (filesize > maxfilesize) {
    maxfilesize = filesize;
  }
  points += 100 + filesize / ((maxfilesize + 1900) / 1900); // gives max 2000 points
  points = points / 2 + (points * (avgspeed / 100)) / (maxavgspeed / 100); // add or remove max 1000 points
  if (racemode) {
    // give points for owning a low percentage of the race on the target
    points += ((100 - fld->getOwnedPercentage()) * 30); // gives max 3000 points
  }
  else {
    // give points for low progress on the target
    int maxprogress = itr->getMaxSiteNumFilesProgress();
    if (maxprogress > 0) {
      points += 3000 - ((fld->getNumUploadedFiles() * 3000) / maxprogress); // gives max 3000 points
    }
  }
  points += prioritypoints; // max 2000 points

  // sfv and nfo files have top priority
  if (f->getExtension().compare("sfv") == 0 ||
      f->getExtension().compare("nfo") == 0) {
    *prio = true;
    return 10000;
  }
  if (points > 10000 || points <= 0) {
    global->getEventLog()->log("Engine", "BUG: unexpected score. Avgspeed: " +
        util::int2Str(avgspeed) + " Maxavgspeed: " + util::int2Str(maxavgspeed) +
        " Filesize: " + util::int2Str(f->getSize()) + " Maxfilesize: " +
        util::int2Str(srs->getMaxFileSize()) + " Ownedpercentage: " +
        util::int2Str(fld->getOwnedPercentage()) + " Maxprogress: " +
        util::int2Str(itr->getMaxSiteNumFilesProgress()));
  }
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<Pointer<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        int avgspeed = its->second->getSite()->getAverageSpeed(itd->second->getSite()->getName());
        if (avgspeed > maxavgspeed) maxavgspeed = avgspeed;
      }
    }
  }
}

int Engine::preparedRaces() const {
  return preparedraces.size();
}

int Engine::currentRaces() const {
  return currentraces.size();
}

int Engine::allRaces() const {
  return allraces.size();
}

int Engine::currentTransferJobs() const {
  return currenttransferjobs.size();
}

int Engine::allTransferJobs() const {
  return alltransferjobs.size();
}

Pointer<Race> Engine::getRace(unsigned int id) const {
  std::list<Pointer<Race> >::const_iterator it;
  for (it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return Pointer<Race>();
}

Pointer<TransferJob> Engine::getTransferJob(unsigned int id) const {
  std::list<Pointer<TransferJob> >::const_iterator it;
  for (it = alltransferjobs.begin(); it != alltransferjobs.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return Pointer<TransferJob>();
}

std::list<Pointer<PreparedRace> >::const_iterator Engine::getPreparedRacesBegin() const {
  return preparedraces.begin();
}

std::list<Pointer<PreparedRace> >::const_iterator Engine::getPreparedRacesEnd() const {
  return preparedraces.end();
}

std::list<Pointer<Race> >::const_iterator Engine::getRacesBegin() const {
  return allraces.begin();
}

std::list<Pointer<Race> >::const_iterator Engine::getRacesEnd() const {
  return allraces.end();
}

std::list<Pointer<TransferJob> >::const_iterator Engine::getTransferJobsBegin() const {
  return alltransferjobs.begin();
}

std::list<Pointer<TransferJob> >::const_iterator Engine::getTransferJobsEnd() const {
  return alltransferjobs.end();
}

void Engine::tick(int message) {
  for (std::list<Pointer<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->checksSinceLastUpdate() >= MAXCHECKSTIMEOUT) {
      if ((*it)->failedTransfersCleared()) {
        global->getEventLog()->log("Engine", "No activity for " + util::int2Str(MAXCHECKSTIMEOUT) +
            " seconds, aborting race: " + (*it)->getName());
        for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator its = (*it)->begin(); its != (*it)->end(); its++) {
          its->second->raceLocalComplete(its->first, 0);
        }
        (*it)->setTimeout();
        issueGlobalComplete(*it);
        currentraces.erase(it);
        break;
      }
      else {
        if ((*it)->clearTransferAttempts()) {
          (*it)->resetUpdateCheckCounter();
        }
      }
    }
  }
  for (std::list<Pointer<TransferJob> >::iterator it = currenttransferjobs.begin(); it != currenttransferjobs.end(); it++) {
    if ((*it)->isDone()) {
      transferJobComplete(*it);
      break;
    }
  }
  std::list<unsigned int> removeids;
  for (std::list<Pointer<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    (*it)->tick();
    if ((*it)->getRemainingTime() < 0) {
      removeids.push_back((*it)->getId());
    }
  }
  while (removeids.size() > 0) {
    unsigned int id = removeids.front();
    removeids.pop_front();
    for (std::list<Pointer<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
      if ((*it)->getId() == id) {
        preparedraces.erase(it);
        break;
      }
    }
  }
  if (!currentraces.size() && !currenttransferjobs.size() && !preparedraces.size() && pokeregistered) {
    global->getTickPoke()->stopPoke(this, 0);
    pokeregistered = false;
  }
}

void Engine::issueGlobalComplete(Pointer<Race> & race) {
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator itd = race->begin(); itd != race->end(); itd++) {
    itd->second->raceGlobalComplete();
  }
}

Pointer<ScoreBoard> Engine::getScoreBoard() const {
  return scoreboard;
}

void Engine::checkStartPoke() {
  if (!pokeregistered) {
    global->getTickPoke()->startPoke(this, "Engine", POKEINTERVAL, 0);
    pokeregistered = true;
  }
}

Pointer<Race> Engine::getCurrentRace(const std::string & release) const {
  for (std::list<Pointer<Race> >::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName() == release) {
      return *it;
    }
  }
  return Pointer<Race>();
}

void Engine::addSiteToRace(Pointer<Race> & race, const std::string & site) {
  SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(site);
  if (!checkBannedGroup(sl->getSite(), race->getGroup())) {
    SiteRace * sr = sl->addRace(race, race->getSection(), race->getName());
    race->addSite(sr, sl);
  }
}

bool Engine::checkBannedGroup(Site * site, const std::string & group) {
  if (site->isBannedGroup(group)) {
    global->getEventLog()->log("Engine", "Ignoring site: " + site->getName() +
        " because the group is banned: " + group);
    return true;
  }
  return false;
}
