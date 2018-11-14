#include "engine.h"

#include <cstdlib>
#include <set>
#include <unordered_set>

#include "core/workmanager.h"
#include "core/tickpoke.h"
#include "scoreboard.h"
#include "scoreboardelement.h"
#include "globalcontext.h"
#include "sitelogic.h"
#include "site.h"
#include "filelist.h"
#include "file.h"
#include "path.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "race.h"
#include "siterace.h"
#include "skiplist.h"
#include "eventlog.h"
#include "sitemanager.h"
#include "sitetransferjob.h"
#include "transferjob.h"
#include "pendingtransfer.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "transferstatus.h"
#include "util.h"
#include "preparedrace.h"
#include "sitetransferjob.h"
#include "sectionmanager.h"

#define SPREAD 0
#define POKEINTERVAL 1000
#define STATICTIMEFORCOMPLETION 5000
#define DIROBSERVETIME 20000
#define SFVDIROBSERVETIME 5000
#define NFO_PRIO_AFTER_SEC 15
#define TICK_NEXTPREPARED_TIMEOUT 600000
#define RETRY_CONNECT_UNTIL_SEC 15

enum EngineTickMessages {
  TICK_MSG_TICKER,
};

Engine::Engine() :
  scoreboard(std::make_shared<ScoreBoard>()),
  maxavgspeed(1024),
  pokeregistered(false),
  nextid(1),
  maxpointsfilesize(2000),
  maxpointsavgspeed(3000),
  maxpointspriority(2500),
  maxpointspercentageowned(2000),
  maxpointslowprogress(2000),
  preparedraceexpirytime(120),
  startnextprepared(false),
  nextpreparedtimeremaining(0)
{
}

Engine::~Engine() {

}

std::shared_ptr<Race> Engine::newSpreadJob(int profile, const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  if (release.empty()) {
    global->getEventLog()->log("Engine", "Spread job skipped due to missing release name.");
    return std::shared_ptr<Race>();
  }
  if (section.empty()) {
    global->getEventLog()->log("Engine", "Spread job skipped due to missing section name.");
    return std::shared_ptr<Race>();
  }
  Section * sectionptr = global->getSectionManager()->getSection(section);
  if (!sectionptr) {
    global->getEventLog()->log("Engine", "Spread job skipped due to undefined section: " + section);
    return std::shared_ptr<Race>();
  }
  std::shared_ptr<Race> race;
  if (profile == SPREAD_PREPARE && startnextprepared) {
    startnextprepared = false;
    profile = SPREAD_RACE;
  }
  bool append = false;
  for (std::list<std::shared_ptr<Race> >::iterator it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == release && (*it)->getSection() == section) {
      race = *it;
      append = true;
      break;
    }
  }
  if (release.find("/") != std::string::npos) {
    global->getEventLog()->log("Engine", "Spread job skipped due to invalid target: " + release);
    return std::shared_ptr<Race>();
  }
  if (!race) {
    race = std::make_shared<Race>(nextid++, static_cast<SpreadProfile>(profile), release, section);
  }
  std::list<std::string> addsites;
  std::list<std::shared_ptr<SiteLogic> > addsiteslogics;
  bool globalskipped = sectionptr->getSkipList().check(release, true, false).action == SKIPLIST_DENY;
  std::list<std::string> skippedsites;
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (!sl) {
      global->getEventLog()->log("Engine", "Trying to use a nonexisting site: " + *it);
      continue;
    }
    if (sl->getSite()->getDisabled()) {
      global->getEventLog()->log("Engine", "Skipping disabled site: " + *it);
      continue;
    }
    if (!sl->getSite()->hasSection(section)) {
      global->getEventLog()->log("Engine", "Trying to use an undefined section: " +
          section + " on " + *it);
      continue;
    }
    SkipListMatch match = sl->getSite()->getSkipList().check((sl->getSite()->getSectionPath(section) / race->getName()).toString(),
                                                             true, false, &sectionptr->getSkipList());
    if (match.action == SKIPLIST_DENY && !sl->getSite()->isAffiliated(race->getGroup())) {
      skippedsites.push_back(sl->getSite()->getName());
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
  if (!addsites.empty()) {
    for (const std::string & skipsite : skippedsites) {
      global->getEventLog()->log("Engine", "Skipping site " + skipsite +
                                 " due to skiplist match: " + release);
      continue;
    }
  }
  else if (globalskipped) {
    global->getEventLog()->log("Engine", "Spread job skipped due to skiplist match: " + release);
    return std::shared_ptr<Race>();
  }
  bool noupload = true;
  bool nodownload = true;
  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = addsiteslogics.begin(); it != addsiteslogics.end(); ++it) {
    if ((*it)->getSite()->getAllowUpload() == SITE_ALLOW_TRANSFER_YES && !(*it)->getSite()->isAffiliated(race->getGroup())) {
      noupload = false;
    }
    if ((*it)->getSite()->getAllowDownload() == SITE_ALLOW_TRANSFER_YES ||
        ((*it)->getSite()->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && (*it)->getSite()->isAffiliated(race->getGroup())))
    {
      nodownload = false;
    }
  }
  if (!append && (noupload || nodownload)) {
    global->getEventLog()->log("Engine", "Ignoring attempt to spread " + release + " in "
        + section + " since no transfers would be performed.");
    return std::shared_ptr<Race>();
  }
  if (addsites.size() < 2 && !append) {
    global->getEventLog()->log("Engine", "Ignoring attempt to spread " + release + " in "
        + section + " on less than 2 sites.");
    return std::shared_ptr<Race>();
  }
  if (addsites.size() > 0 || append) {
    checkStartPoke();
    if (profile == SPREAD_PREPARE) {
      global->getEventLog()->log("Engine", "Preparing spread job: " + section + "/" + release +
                " on " + util::int2Str((int)addsites.size()) + " sites.");
      preparedraces.push_back(std::make_shared<PreparedRace>(race->getId(), release, section, addsites, preparedraceexpirytime));
      for (std::list<std::shared_ptr<SiteLogic> >::const_iterator ait = addsiteslogics.begin(); ait != addsiteslogics.end(); ait++) {
        (*ait)->activateAll();
      }
      return std::shared_ptr<Race>();
    }
    bool readdtocurrent = true;
    if (append) {
      for (std::list<std::shared_ptr<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
        if (*it == race) {
          readdtocurrent = false;
          break;
        }
      }
      if (readdtocurrent) {
        global->getEventLog()->log("Engine", "Reactivating spread job: " + section + "/" + release);
        race->setUndone();
        for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
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
      global->getEventLog()->log("Engine", "Starting spread job: " + section + "/" + release +
          " on " + util::int2Str((int)addsites.size()) + " sites.");
      sectionptr->addJob();
      global->getStatistics()->addSpreadJob();
    }
    else {
      if (addsites.size()) {
        global->getEventLog()->log("Engine", "Appending to spread job: " + section + "/" + release +
            " with " + util::int2Str((int)addsites.size()) + " site" + (addsites.size() > 1 ? "s" : "") + ".");
      }
      if (readdtocurrent) {
        currentraces.push_back(race);
      }
    }
    setSpeedScale();
    preSeedPotentialData(race);
  }
  return race;
}

std::shared_ptr<Race> Engine::newSpreadJob(int profile, const std::string & release, const std::string & section) {
  std::list<std::string> sites;
  for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
    if ((*it)->hasSection(section) && !(*it)->getDisabled()) {
      sites.push_back((*it)->getName());
    }
  }
  return newSpreadJob(profile, release, section, sites);
}

std::shared_ptr<Race> Engine::newRace(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  return newSpreadJob(SPREAD_RACE, release, section, sites);
}

std::shared_ptr<Race> Engine::newRace(const std::string & release, const std::string & section) {
  return newSpreadJob(SPREAD_RACE, release, section);
}

std::shared_ptr<Race> Engine::newDistribute(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  return newSpreadJob(SPREAD_DISTRIBUTE, release, section, sites);
}

std::shared_ptr<Race> Engine::newDistribute(const std::string & release, const std::string & section) {
  return newSpreadJob(SPREAD_DISTRIBUTE, release, section);
}

void Engine::prepareRace(const std::string & release, const std::string & section, const std::list<std::string> & sites) {
  newSpreadJob(SPREAD_PREPARE, release, section, sites);
}

void Engine::prepareRace(const std::string & release, const std::string & section) {
  newSpreadJob(SPREAD_PREPARE, release, section);
}

void Engine::startPreparedRace(unsigned int id) {
  for (std::list<std::shared_ptr<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    if ((*it)->getId() == id) {
      newRace((*it)->getRelease(), (*it)->getSection(), (*it)->getSites());
      preparedraces.erase(it);
      return;
    }
  }
}

void Engine::deletePreparedRace(unsigned int id) {
  for (std::list<std::shared_ptr<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    if ((*it)->getId() == id) {
      preparedraces.erase(it);
      return;
    }
  }
}

void Engine::startLatestPreparedRace() {
  if (preparedraces.size()) {
    std::shared_ptr<PreparedRace> preparedrace = preparedraces.back();
    preparedraces.pop_back();
    newRace(preparedrace->getRelease(), preparedrace->getSection(), preparedrace->getSites());
  }
}

void Engine::toggleStartNextPreparedRace() {
  if (!startnextprepared) {
    startnextprepared = true;
    nextpreparedtimeremaining = TICK_NEXTPREPARED_TIMEOUT;
    checkStartPoke();
    global->getEventLog()->log("Engine", "Enabling next prepared spread job starter");
  }
  else {
    startnextprepared = false;
    global->getEventLog()->log("Engine", "Disabling next prepared spread job starter");
  }
}

bool Engine::getNextPreparedRaceStarterEnabled() const {
  return startnextprepared;
}

int Engine::getNextPreparedRaceStarterTimeRemaining() const {
  return nextpreparedtimeremaining / 1000;
}

unsigned int Engine::newTransferJobDownload(const std::string & srcsite, FileList * srcfilelist, const std::string & file, const Path & dstpath) {
  return newTransferJobDownload(srcsite, srcfilelist, file, dstpath, file);
}

unsigned int Engine::newTransferJobUpload(const Path & srcpath, const std::string & file, const std::string & dstsite, FileList * dstfilelist) {
  return newTransferJobUpload(srcpath, file, dstsite, dstfilelist, file);
}

unsigned int Engine::newTransferJobFXP(const std::string & srcsite, FileList * srcfilelist, const std::string & dstsite, FileList * dstfilelist, const std::string & file) {
  return newTransferJobFXP(srcsite, srcfilelist, file, dstsite, dstfilelist, file);
}

unsigned int Engine::newTransferJobDownload(const std::string & srcsite, FileList * srcfilelist, const std::string & srcfile, const Path & dstpath, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(srcsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, sl, srcfilelist, srcfile, dstpath, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting download job: " + srcfile +
            " from " + srcsite);
  sl->addTransferJob(tj->getSrcTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

unsigned int Engine::newTransferJobDownload(const std::string & srcsite, const Path & srcpath, const std::string & srcfile, const Path & dstpath, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(srcsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, sl, srcpath, srcfile, dstpath, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting download job: " + srcfile +
            " from " + srcsite);
  sl->addTransferJob(tj->getSrcTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

unsigned int Engine::newTransferJobUpload(const Path & srcpath, const std::string & srcfile, const std::string & dstsite, FileList * dstfilelist, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, srcpath, srcfile, sl, dstfilelist, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting upload job: " + srcfile +
            " to " + dstsite);
  sl->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

unsigned int Engine::newTransferJobUpload(const Path & srcpath, const std::string & srcfile, const std::string & dstsite, const Path & dstpath, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, srcpath, srcfile, sl, dstpath, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting upload job: " + srcfile +
            " to " + dstsite);
  sl->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

unsigned int Engine::newTransferJobFXP(const std::string & srcsite, FileList * srcfilelist, const std::string & srcfile, const std::string & dstsite, FileList * dstfilelist, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> slsrc = global->getSiteLogicManager()->getSiteLogic(srcsite);
  const std::shared_ptr<SiteLogic> sldst = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!slsrc) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (!sldst) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, slsrc, srcfilelist, srcfile, sldst, dstfilelist, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting FXP job: " + srcfile +
            " - " + srcsite + " -> " + dstsite);
  slsrc->addTransferJob(tj->getSrcTransferJob());
  sldst->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

unsigned int Engine::newTransferJobFXP(const std::string & srcsite, const Path & srcpath, const std::string & srcfile, const std::string & dstsite, const Path & dstpath, const std::string & dstfile) {
  const std::shared_ptr<SiteLogic> slsrc = global->getSiteLogicManager()->getSiteLogic(srcsite);
  const std::shared_ptr<SiteLogic> sldst = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!slsrc) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (!sldst) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, slsrc, srcpath, srcfile, sldst, dstpath, dstfile);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting FXP job: " + srcfile +
            " - " + srcsite + " -> " + dstsite);
  slsrc->addTransferJob(tj->getSrcTransferJob());
  sldst->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  return id;
}

void Engine::removeSiteFromRace(std::shared_ptr<Race> & race, const std::string & site) {
  if (!!race) {
    SiteRace * sr = race->getSiteRace(site);
    if (sr != NULL) {
      const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
      race->removeSite(sr);
      if (!!sl) {
        sl->removeRace(race->getId());
      }
    }
  }
}

void Engine::removeSiteFromRaceDeleteFiles(std::shared_ptr<Race> & race, const std::string & site, bool allfiles) {
  if (!!race) {
    SiteRace * sr = race->getSiteRace(site);
    if (sr != NULL) {
      const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
      race->removeSite(sr);
      if (!!sl) {
        sl->requestDelete(sr->getPath(), true, false, allfiles);
        sl->removeRace(race->getId());
      }
    }
  }
}

void Engine::abortRace(std::shared_ptr<Race> & race) {
  if (!!race) {
    race->abort();
    for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
      it->second->abortRace(race->getId());
    }
    currentraces.remove(race);
    global->getEventLog()->log("Engine", "Spread job aborted: " + race->getName());
  }
}

void Engine::resetRace(std::shared_ptr<Race> & race, bool hard) {
  if (!!race) {
    race->reset();
    for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
      if (hard) {
        it->first->hardReset();
      }
      else {
        it->first->softReset();
      }
      it->second->activateAll();
    }
    bool current = false;
    for (std::list<std::shared_ptr<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
      if (*it == race) {
        current = true;
        break;
      }
    }
    if (!current) {
      currentraces.push_back(race);
    }
    checkStartPoke();
    global->getEventLog()->log("Engine", "Spread job reset: " + race->getName());
  }
}

void Engine::deleteOnAllSites(std::shared_ptr<Race> & race) {
  std::list<std::shared_ptr<Site> > sites;
  for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
    sites.push_back(it->second->getSite());
  }
  deleteOnSites(race, sites);
}

void Engine::deleteOnSites(std::shared_ptr<Race> & race, std::list<std::shared_ptr<Site> > delsites) {
  deleteOnSites(race, delsites, true);
}

void Engine::deleteOnSites(std::shared_ptr<Race> & race, std::list<std::shared_ptr<Site> > delsites, bool allfiles) {
  if (!!race) {
    if (race->getStatus() == RACE_STATUS_RUNNING) {
      abortRace(race);
    }
    std::string sites;
    for (std::list<std::shared_ptr<Site> >::const_iterator it = delsites.begin(); it != delsites.end(); it++) {
      if (!*it) {
        continue;
      }
      std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
      if (!sl) {
        continue;
      }
      SiteRace * sr = sl->getRace(race->getName());
      if (!sr) {
        global->getEventLog()->log("Engine", "Site " + (*it)->getName() + " is not in spread job: " + race->getName());
        continue;
      }
      const Path & path = sr->getPath();
      sl->requestDelete(path, true, false, allfiles);
      sites += (*it)->getName() + ",";
    }
    if (sites.length() > 0) {
      sites = sites.substr(0, sites.length() - 1);
      global->getEventLog()->log("Engine", std::string("Attempting delete of ") + (allfiles ? "all" : "own") + " files in " + race->getName() + " on: " + sites);
    }
  }
}

void Engine::abortTransferJob(std::shared_ptr<TransferJob> & tj) {
  tj->abort();
  currenttransferjobs.remove(tj);
  global->getEventLog()->log("Engine", "Transfer job aborted: " + tj->getSrcFileName());
}

void Engine::jobFileListRefreshed(SiteLogic * sls, CommandOwner * commandowner) {
  switch (commandowner->classType()) {
    case COMMANDOWNER_SITERACE: {
      SiteRace * sr = static_cast<SiteRace *>(commandowner);
      std::shared_ptr<Race> race = sr->getRace();
      if (currentraces.size() > 0) {
        if (!global->getWorkManager()->overload()) {
          estimateRaceSize(race);
          checkIfRaceComplete(sls, race);
          filelistUpdated();
        }
        else {
          ++dropped;
        }
      }
      break;
    }
    case COMMANDOWNER_TRANSFERJOB: {
      SiteTransferJob * stj = static_cast<SiteTransferJob *>(commandowner);
      std::shared_ptr<TransferJob> tj = getTransferJob(stj->getId());
      refreshPendingTransferList(tj);
      break;
    }
  }
}

void Engine::filelistUpdated() {
  if (currentraces.size() > 0) {
    if (!global->getWorkManager()->overload()) {
      refreshScoreBoard();
      issueOptimalTransfers();
    }
    else {
      ++dropped;
    }
  }
}

bool Engine::transferJobActionRequest(std::shared_ptr<SiteTransferJob> & stj) {
  std::shared_ptr<TransferJob> tj = getTransferJob(stj->getTransferJob()->getId());
  if (tj->getType() == TRANSFERJOB_FXP && stj->otherWantsList()) {
    stj->getOtherSiteLogic()->haveConnectedActivate(1);
    return false;
  }
  std::unordered_map<std::shared_ptr<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
    it = pendingtransfers.find(tj);
  }
  refreshPendingTransferList(tj);
  if (it->second.size() == 0) {
    bool action = tj->refreshOrAlmostDone();
    if (tj->getType() == TRANSFERJOB_FXP && stj->otherWantsList()) {
      stj->getOtherSiteLogic()->haveConnectedActivate(1);
      return false;
    }
    return action;
  }
  tj->clearRefreshLists();
  PendingTransfer pt = it->second.front();
  switch (pt.type()) {
    case PENDINGTRANSFER_DOWNLOAD:
    {
      if (!pt.getSrc()->downloadSlotAvailable()) return false;
      std::shared_ptr<TransferStatus> ts = global->getTransferManager()->suggestDownload(pt.getSrcFileName(),
          pt.getSrc(), pt.getSrcFileList(), pt.getLocalFileList(), tj->getSrcTransferJob().get());
      tj->addTransfer(ts);
      break;
    }
    case PENDINGTRANSFER_UPLOAD:
    {
      if (!pt.getDst()->uploadSlotAvailable()) return false;
      std::shared_ptr<TransferStatus> ts = global->getTransferManager()->suggestUpload(pt.getSrcFileName(),
          pt.getLocalFileList(), pt.getDst(), pt.getDstFileList(), tj->getDstTransferJob().get());
      tj->addTransfer(ts);
      break;
    }
    case PENDINGTRANSFER_FXP:
    {
      if (!pt.getSrc()->downloadSlotAvailable()) return false;
      if (!pt.getDst()->uploadSlotAvailable()) return false;
      if (pt.getDst() == pt.getSrc() && pt.getDst()->slotsAvailable() < 2) {
        pt.getDst()->haveConnectedActivate(2);
        return false;
      }
      std::shared_ptr<TransferStatus> ts = global->getTransferManager()->suggestTransfer(pt.getSrcFileName(),
          pt.getSrc(), pt.getSrcFileList(), pt.getDst(), pt.getDstFileList(), tj->getSrcTransferJob().get(), tj->getDstTransferJob().get());
      tj->addTransfer(ts);
      break;
    }
  }
  it->second.pop_front();
  if (tj->getStatus() == TRANSFERJOB_QUEUED) {
    tj->start();
  }
  return true;
}

void Engine::raceActionRequest() {
  issueOptimalTransfers();
}

void Engine::estimateRaceSizes() {
  for (std::list<std::shared_ptr<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    estimateRaceSize(*itr, true);
  }
}

void Engine::estimateRaceSize(const std::shared_ptr<Race> & race) {
  estimateRaceSize(race, false);
}

void Engine::estimateRaceSize(const std::shared_ptr<Race> & race, bool forceupdate) {
  for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator its = race->begin(); its != race->end(); its++) {
    SiteRace * srs = its->first;
    const SkipList & siteskiplist = its->second->getSite()->getSkipList();
    const SkipList & sectionskiplist = race->getSectionSkipList();
    std::unordered_map<std::string, FileList *>::const_iterator itfl;
    for (itfl = srs->fileListsBegin(); itfl != srs->fileListsEnd(); itfl++) {
      FileList * fls = itfl->second;
      if (!forceupdate && srs->sizeEstimated(fls)) {
        continue;
      }
      reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, false);
      if (fls->hasSFV()) {
        if (srs->getSFVObservedTime(fls) > SFVDIROBSERVETIME) {
          reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, true);
        }
      }
      else {
        if (srs->getObservedTime(fls) > DIROBSERVETIME) {
          reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, true);
        }
      }
    }
  }
}

bool setContainsPattern(const std::unordered_set<std::string> & uniques, const std::string & matchpattern) {
  for (const std::string & unique : uniques) {
    if (util::wildcmp(matchpattern.c_str(), unique.c_str())) {
      return true;
    }
  }
  return false;
}

void Engine::reportCurrentSize(const SkipList & siteskiplist, const SkipList & sectionskiplist, SiteRace * srs, FileList * fls, bool final) {
  std::unordered_set<std::string> uniques;
  std::unordered_map<std::string, File *>::const_iterator itf;
  std::string subpath = srs->getSubPathForFileList(fls);
  for (itf = fls->begin(); itf != fls->end(); itf++) {
    File * file = itf->second;
    if (file->isDirectory()) {
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
    Path prepend = subpath;
    SkipListMatch match = siteskiplist.check((prepend / filename).toString(), false, true, &sectionskiplist);
    if (match.action == SKIPLIST_DENY || (match.action == SKIPLIST_UNIQUE && setContainsPattern(uniques, match.matchpattern))) {
      continue;
    }
    uniques.insert(filename);
  }
  srs->reportSize(fls, uniques, final);
}

void Engine::refreshScoreBoard() {
  scoreboard->wipe();
  for (std::list<std::shared_ptr<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    std::shared_ptr<Race> race = *itr;
    const SkipList & secskip = race->getSectionSkipList();
    bool racemode = race->getProfile() == SPREAD_RACE;
    for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator its = race->begin(); its != race->end(); its++) {
      SiteRace * srs = its->first;
      const std::shared_ptr<SiteLogic> & sls = its->second;
      const std::shared_ptr<Site> & srcsite = sls->getSite();
      if (!sls->getCurrLogins() ||
          srcsite->getAllowDownload() == SITE_ALLOW_TRANSFER_NO ||
          (srcsite->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && !srcsite->isAffiliated(race->getGroup())))
      {
        continue;
      }
      for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator itd = race->begin(); itd != race->end(); itd++) {
        SiteRace * srd = itd->first;
        FileList * fldroot = srd->getFileListForPath("");
        const std::shared_ptr<SiteLogic> & sld = itd->second;
        const std::shared_ptr<Site> & dstsite = sld->getSite();
        const SkipList & dstskip = dstsite->getSkipList();
        if (!raceTransferPossible(sls, sld, race)) continue;
        if (!sld->getCurrLogins()) continue;
        int avgspeed = srcsite->getAverageSpeed(dstsite->getName());
        if (avgspeed > maxavgspeed) {
          avgspeed = maxavgspeed;
        }
        int prioritypoints = getPriorityPoints(dstsite->getPriority());
        for (std::unordered_map<std::string, FileList *>::const_iterator itfls = srs->fileListsBegin(); itfls != srs->fileListsEnd(); itfls++) {
          if (!itfls->first.empty()) {
            SkipListMatch dirmatch = dstskip.check(itfls->first, true, true, &secskip);
            if (dirmatch.action == SKIPLIST_DENY ||
                (dirmatch.action == SKIPLIST_UNIQUE &&
                 fldroot->containsPatternBefore(dirmatch.matchpattern, true, itfls->first)))
            {
              continue;
            }
          }
          FileList * fls = itfls->second;
          FileList * fld = srd->getFileListForPath(itfls->first);
          if (fld != NULL) {
            if (fld->getState() != FILELIST_LISTED && fld->getState() != FILELIST_NONEXISTENT) continue;
            std::unordered_map<std::string, File *>::const_iterator itf;
            for (itf = fls->begin(); itf != fls->end(); itf++) {
              File * f = itf->second;
              const std::string filename = f->getName();
              if (fld->contains(filename) || f->isDirectory() || f->getSize() == 0) continue;
              const Path & prepend = itfls->first;
              SkipListMatch filematch = dstskip.check((prepend / itf->first).toString(), false, true, &secskip);
              if (filematch.action == SKIPLIST_DENY || (filematch.action == SKIPLIST_UNIQUE &&
                                                        fld->containsPatternBefore(filematch.matchpattern, false, filename))) {
                continue;
              }
              if (race->hasFailedTransfer(f, fls, fld)) continue;
              bool prio = false;
              unsigned short score = calculateScore(f, race, fls, srs, fld, srd, avgspeed, &prio, prioritypoints, racemode);
              scoreboard->add(filename, score, prio, sls, fls, srs, sld, fld, srd, race, itfls->first);
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
  scoreboard->shuffleEquals();
}

void Engine::refreshPendingTransferList(std::shared_ptr<TransferJob> & tj) {
  std::unordered_map<std::shared_ptr<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
  }
  it = pendingtransfers.find(tj);
  std::list<PendingTransfer> & list = it->second;
  list.clear();
  tj->clearExisting();
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD: {
      std::unordered_map<std::string, FileList *>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        FileList * srclist = it2->second;
        std::shared_ptr<LocalFileList> dstlist = tj->findLocalFileList(it2->first);
        for (std::unordered_map<std::string, File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          if ((it2->first != "" || srcit->first == tj->getSrcFileName()) &&
              !srcit->second->isDirectory() && srcit->second->getSize() > 0)
          {
            std::unordered_map<std::string, LocalFile>::const_iterator dstit;
            if (!dstlist) {
              dstlist = tj->wantedLocalDstList(it2->first);
            }
            dstit = dstlist->find(srcit->first);
            if (tj->hasFailedTransfer((Path(dstlist->getPath()) / srcit->first).toString())) {
              continue;
            }
            const Path subpath = it2->first;
            if (!dstlist || dstit == dstlist->end() || dstit->second.getSize() == 0) {
              PendingTransfer p(tj->getSrc(), srclist, srcit->first, dstlist, srcit->first);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath / srcit->first, srcit->second->getSize());
            }
            else {
              tj->targetExists(subpath / srcit->first);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_UPLOAD: {
      std::unordered_map<std::string, std::shared_ptr<LocalFileList> >::const_iterator lit;
      for (lit = tj->localFileListsBegin(); lit != tj->localFileListsEnd(); lit++) {
        FileList * dstlist = tj->findDstList(lit->first);
        if (dstlist == NULL || dstlist->getState() == FILELIST_UNKNOWN || dstlist->getState() == FILELIST_EXISTS) {
          continue;
        }
        for (std::unordered_map<std::string, LocalFile>::const_iterator lfit = lit->second->begin(); lfit != lit->second->end(); lfit++) {
          if ((lit->first != "" || lfit->first == tj->getSrcFileName()) &&
              !lfit->second.isDirectory() && lfit->second.getSize() > 0)
          {
            std::string filename = lfit->first;
            const Path subpath = lit->first;
            if (dstlist->getFile(filename) == NULL) {
              if (tj->hasFailedTransfer((Path(dstlist->getPath()) / filename).toString())) {
                continue;
              }
              PendingTransfer p(lit->second, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath / filename, lfit->second.getSize());
            }
            else {
              tj->targetExists(subpath / filename);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_FXP: {
      std::unordered_map<std::string, FileList *>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        FileList * srclist = it2->second;
        FileList * dstlist = tj->findDstList(it2->first);
        if (dstlist == NULL || dstlist->getState() == FILELIST_UNKNOWN || dstlist->getState() == FILELIST_EXISTS) {
          continue;
        }
        for (std::unordered_map<std::string, File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          if ((it2->first != "" || srcit->first == tj->getSrcFileName()) &&
              !srcit->second->isDirectory() && srcit->second->getSize() > 0)
          {
            std::string filename = srcit->first;
            const Path subpath = it2->first;
            if (dstlist->getFile(filename) == NULL) {
              if (tj->hasFailedTransfer((Path(dstlist->getPath()) / filename).toString())) {
                continue;
              }
              PendingTransfer p(tj->getSrc(), srclist, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);

              tj->addPendingTransfer(subpath / filename, srcit->second->getSize());
            }
            else {
              tj->targetExists(subpath / filename);
            }
          }
        }
      }
      break;
    }
  }
  tj->updateStatus();
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
  std::shared_ptr<SiteLogic> sls;
  std::shared_ptr<SiteLogic> sld;
  std::string filename;
  std::shared_ptr<Race> race;
  for (it = scoreboard->begin(); it != scoreboard->end(); it++) {
    sbe = *it;
    sls = sbe->getSource();
    sld = sbe->getDestination();
    race = sbe->getRace();
    filename = sbe->fileName();
    if (sbe->getDestinationFileList()->contains(filename)) {
      continue;
    }
    //potentiality handling
    if (!sbe->isPrioritized()) { // priority files shouldn't affect the potential tracking
      sls->pushPotential(sbe->getScore(), filename, sld);
    }
    if (!sls->downloadSlotAvailable()) {
      continue;
    }
    if (!sld->uploadSlotAvailable()) {
      continue;
    }
    if (!sls->potentialCheck(sbe->getScore())) {
      continue;
    }
    if (sbe->wasAttempted()) {
      continue;
    }
    if (!sbe->getSourceFileList()->getFile(filename)) {
      continue;
    }
    if (sbe->getDestinationSiteRace()->isAborted()) {
      continue;
    }
    SkipListMatch match = sbe->getDestination()->getSite()->getSkipList().check(
        (Path(sbe->subDir()) / filename).toString(), false, true, &race->getSectionSkipList());
    if (match.action == SKIPLIST_UNIQUE &&
        sbe->getDestinationFileList()->containsPatternBefore(match.matchpattern, false, filename))
    {
      continue;
    }
    std::shared_ptr<TransferStatus> ts =
      global->getTransferManager()->suggestTransfer(filename, sls,
        sbe->getSourceFileList(), sld, sbe->getDestinationFileList(),
        sbe->getSourceSiteRace(), sbe->getDestinationSiteRace());
    race->addTransfer(ts);
    sbe->setAttempted();
  }
}

void Engine::checkIfRaceComplete(SiteLogic * sls, std::shared_ptr<Race> & race) {
  SiteRace * srs = sls->getRace(race->getName());
  if (!srs->isDone()) {
    bool unfinisheddirs = false;
    bool emptydirs = false;
    int completedlists = 0;
    std::unordered_set<std::string> subpaths = race->getSubPaths();
    for (std::unordered_set<std::string>::iterator itsp = subpaths.begin(); itsp != subpaths.end(); itsp++) {
      FileList * spfl = srs->getFileListForPath(*itsp);
      if (spfl != NULL && spfl->getState() == FILELIST_LISTED) {
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
          !spfl->hasFilesUploading())
        {
          completedlists++;
          if (!srs->isSubPathComplete(spfl)) {
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
        for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
          if (it->second.get() != sls && !it->first->isDone()) {
            uploadslotcount += it->second->getSite()->getMaxUp();
          }
        }
        sls->raceLocalComplete(srs, uploadslotcount);
        global->getEventLog()->log("Engine", "Spread job " + race->getName() + " completed on " +
            sls->getSite()->getName());
      }
      if (race->isDone()) {
        raceComplete(race);
      }
    }
  }
}

void Engine::raceComplete(std::shared_ptr<Race> race) {
  issueGlobalComplete(race);
  for (std::list<std::shared_ptr<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it) == race) {
      currentraces.erase(it);
      break;
    }
  }
  refreshScoreBoard();
  global->getEventLog()->log("Engine", "Spread job globally completed: " + race->getName());
  if (dropped) {
    global->getEventLog()->log("Engine", "Scoreboard refreshes dropped since spread job start: " + util::int2Str(dropped));
  }
  return;
}

void Engine::transferJobComplete(std::shared_ptr<TransferJob> tj) {

  for (std::list<std::shared_ptr<TransferJob> >::iterator it = currenttransferjobs.begin(); it != currenttransferjobs.end(); it++) {
    if ((*it) == tj) {
      currenttransferjobs.erase(it);
      break;
    }
  }
  global->getEventLog()->log("Engine", tj->typeString() + " job complete: " + tj->getSrcFileName());
}

unsigned short Engine::calculateScore(File * f, std::shared_ptr<Race> & itr, FileList * fls, SiteRace * srs,
                                      FileList * fld, SiteRace * srd, int avgspeed, bool * prio,
                                      int prioritypoints, bool racemode) const
{
  // sfv and nfo files have top priority
  if (f->getExtension().compare("sfv") == 0 ||
      (f->getExtension().compare("nfo") == 0 && itr->getTimeSpent() > NFO_PRIO_AFTER_SEC))
  {
    *prio = true;
    return 10000;
  }

  unsigned short points = 0;
  unsigned long long int filesize = f->getSize();
  unsigned long long int maxfilesize = srs->getMaxFileSize();
  if (avgspeed > maxavgspeed) {
    avgspeed = maxavgspeed;
  }
  if (filesize > maxfilesize) {
    maxfilesize = filesize;
  }

  if (maxfilesize) {
    unsigned long long int pointsfilesize = maxpointsfilesize;
    pointsfilesize *= filesize;
    pointsfilesize /= maxfilesize;
    points += pointsfilesize;
  }

  points += getSpeedPoints(avgspeed);

  if (racemode) {
    unsigned long long int pointspercentageowned = maxpointspercentageowned;
    int unownedpercentage = 100 - fld->getOwnedPercentage();
    pointspercentageowned *= unownedpercentage;
    pointspercentageowned /= 100;
    points += pointspercentageowned;
  }
  else {
    unsigned long long int pointslowprogress = maxpointslowprogress;
    int maxprogress = itr->getMaxSiteNumFilesProgress();
    if (maxprogress) {
      pointslowprogress *= fld->getNumUploadedFiles();
      pointslowprogress /= maxprogress;
      points += pointslowprogress;
    }
  }

  points += prioritypoints;
  util::assert(points >= 0 && points < 10000);
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<std::shared_ptr<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        int avgspeed = its->second->getSite()->getAverageSpeed(itd->second->getSite()->getName());
        if (avgspeed > maxavgspeed) maxavgspeed = avgspeed;
      }
    }
  }
}

void Engine::preSeedPotentialData(std::shared_ptr<Race> & race) {
  std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator srcit;
  std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator dstit;
  int maxpointssizeandowned = getMaxPointsFileSize() + getMaxPointsPercentageOwned();
  for (srcit = race->begin(); srcit != race->end(); srcit++) {
    const std::shared_ptr<SiteLogic> & sls = srcit->second;
    if (sls->getSite()->getAllowDownload() == SITE_ALLOW_TRANSFER_NO ||
        (sls->getSite()->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && !sls->getSite()->isAffiliated(race->getGroup())))
    {
      continue;
    }
    for (dstit = race->begin(); dstit != race->end(); dstit++) {
      const std::shared_ptr<SiteLogic> & sld = dstit->second;
      if (!raceTransferPossible(sls, sld, race)) continue;
      int priopoints = getPriorityPoints(sld->getSite()->getPriority());
      int speedpoints = getSpeedPoints(sls->getSite()->getAverageSpeed(sld->getSite()->getName()));
      for (unsigned int i = 0; i < sld->getSite()->getMaxUp(); ++i) {
        sls->pushPotential(maxpointssizeandowned + priopoints + speedpoints, "preseed", sld);
      }
    }
  }
}

bool Engine::raceTransferPossible(const std::shared_ptr<SiteLogic> & sls, const std::shared_ptr<SiteLogic> & sld, std::shared_ptr<Race> & race) const {
  if (sls == sld) return false;
  const std::shared_ptr<Site> & srcsite = sls->getSite();
  const std::shared_ptr<Site> & dstsite = sld->getSite();
  if (dstsite->getAllowUpload() == SITE_ALLOW_TRANSFER_NO) return false;
  if (!srcsite->isAllowedTargetSite(dstsite)) return false;
  if (dstsite->isAffiliated(race->getGroup()) && race->getProfile() != SPREAD_DISTRIBUTE) return false;
  if (srcsite->hasBrokenPASV() &&
      dstsite->hasBrokenPASV()) return false;
  //ssl check
  int srcpolicy = srcsite->getSSLTransferPolicy();
  int dstpolicy = dstsite->getSSLTransferPolicy();
  if ((srcpolicy == SITE_SSL_ALWAYS_OFF &&
       dstpolicy == SITE_SSL_ALWAYS_ON) ||
      (srcpolicy == SITE_SSL_ALWAYS_ON &&
       dstpolicy == SITE_SSL_ALWAYS_OFF))
  {
    return false;
  }
  if (srcpolicy == SITE_SSL_ALWAYS_ON && dstpolicy == SITE_SSL_ALWAYS_ON &&
      !srcsite->supportsSSCN() && !dstsite->supportsSSCN())
  {
    return false;
  }
  return true;
}

unsigned int Engine::preparedRaces() const {
  return preparedraces.size();
}

unsigned int Engine::currentRaces() const {
  return currentraces.size();
}

unsigned int Engine::allRaces() const {
  return allraces.size();
}

unsigned int Engine::currentTransferJobs() const {
  return currenttransferjobs.size();
}

unsigned int Engine::allTransferJobs() const {
  return alltransferjobs.size();
}

std::shared_ptr<Race> Engine::getRace(unsigned int id) const {
  std::list<std::shared_ptr<Race> >::const_iterator it;
  for (it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return std::shared_ptr<Race>();
}

std::shared_ptr<Race> Engine::getRace(const std::string & race) const {
  std::list<std::shared_ptr<Race> >::const_iterator it;
  for (it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == race) {
      return *it;
    }
  }
  return std::shared_ptr<Race>();
}

std::shared_ptr<TransferJob> Engine::getTransferJob(unsigned int id) const {
  std::list<std::shared_ptr<TransferJob> >::const_iterator it;
  for (it = alltransferjobs.begin(); it != alltransferjobs.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return std::shared_ptr<TransferJob>();
}

std::list<std::shared_ptr<PreparedRace> >::const_iterator Engine::getPreparedRacesBegin() const {
  return preparedraces.begin();
}

std::list<std::shared_ptr<PreparedRace> >::const_iterator Engine::getPreparedRacesEnd() const {
  return preparedraces.end();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getRacesBegin() const {
  return allraces.begin();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getRacesEnd() const {
  return allraces.end();
}

std::list<std::shared_ptr<TransferJob> >::const_iterator Engine::getTransferJobsBegin() const {
  return alltransferjobs.begin();
}

std::list<std::shared_ptr<TransferJob> >::const_iterator Engine::getTransferJobsEnd() const {
  return alltransferjobs.end();
}

void Engine::tick(int message) {
  if (startnextprepared) {
    nextpreparedtimeremaining -= POKEINTERVAL;
    if (nextpreparedtimeremaining <= 0) {
      startnextprepared = false;
      global->getEventLog()->log("Engine", "Next prepared spread job starter timed out.");
    }
  }
  for (std::list<std::shared_ptr<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    int timeoutafterseconds = (*it)->timeoutCheck();
    if (timeoutafterseconds != -1) {
      if ((*it)->failedTransfersCleared()) {
        global->getEventLog()->log("Engine", "No activity for " + std::to_string(timeoutafterseconds) +
            " seconds, aborting spread job: " + (*it)->getName());
        for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator its = (*it)->begin(); its != (*it)->end(); its++) {
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
    for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++) {
      int wantedlogins = it2->second->getSite()->getMaxDown();
      if (!it2->first->isDone()) {
        wantedlogins = it2->second->getSite()->getMaxLogins();
      }
      if (it2->second->getCurrLogins() + it2->second->getCleanlyClosedConnectionsCount() < wantedlogins &&
          (it2->first->getMaxFileSize() || (*it)->getTimeSpent() < RETRY_CONNECT_UNTIL_SEC))
      {
        it2->second->activateAll();
      }
    }
  }
  for (std::list<std::shared_ptr<TransferJob> >::const_iterator it = currenttransferjobs.begin(); it != currenttransferjobs.end(); it++) {
    if ((*it)->isDone()) {
      transferJobComplete(*it);
      break;
    }
    else {
      if (!!(*it)->getSrc() && !(*it)->getSrc()->getCurrLogins()) {
        (*it)->getSrc()->haveConnectedActivate((*it)->maxSlots());
      }
      if (!!(*it)->getDst() && !(*it)->getDst()->getCurrLogins()) {
        (*it)->getDst()->haveConnectedActivate((*it)->maxSlots());
      }
    }
  }
  std::list<unsigned int> removeids;
  for (std::list<std::shared_ptr<PreparedRace> >::const_iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    (*it)->tick();
    if ((*it)->getRemainingTime() < 0) {
      removeids.push_back((*it)->getId());
    }
  }
  while (removeids.size() > 0) {
    unsigned int id = removeids.front();
    removeids.pop_front();
    for (std::list<std::shared_ptr<PreparedRace> >::iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
      if ((*it)->getId() == id) {
        preparedraces.erase(it);
        break;
      }
    }
  }
  if (!currentraces.size() && !currenttransferjobs.size() && !preparedraces.size() && !startnextprepared && pokeregistered) {
    global->getTickPoke()->stopPoke(this, 0);
    pokeregistered = false;
  }
  estimateRaceSizes();
}

void Engine::issueGlobalComplete(std::shared_ptr<Race> & race) {
  for (std::set<std::pair<SiteRace *, std::shared_ptr<SiteLogic> > >::const_iterator itd = race->begin(); itd != race->end(); itd++) {
    itd->second->raceGlobalComplete();
  }
}

std::shared_ptr<ScoreBoard> Engine::getScoreBoard() const {
  return scoreboard;
}

void Engine::checkStartPoke() {
  if (!pokeregistered) {
    global->getTickPoke()->startPoke(this, "Engine", POKEINTERVAL, TICK_MSG_TICKER);
    pokeregistered = true;
  }
}

std::shared_ptr<Race> Engine::getCurrentRace(const std::string & release) const {
  for (std::list<std::shared_ptr<Race> >::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName() == release) {
      return *it;
    }
  }
  return std::shared_ptr<Race>();
}

void Engine::addSiteToRace(std::shared_ptr<Race> & race, const std::string & site) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
  if (sl->getSite()->getSkipList().check((sl->getSite()->getSectionPath(race->getSection()) / race->getName()).toString(),
                                         true, false, &race->getSectionSkipList()).action != SKIPLIST_DENY ||
      sl->getSite()->isAffiliated(race->getGroup()))
  {
    SiteRace * sr = sl->addRace(race, race->getSection(), race->getName());
    race->addSite(sr, sl);
  }
}

int Engine::getMaxPointsRaceTotal() const {
  return getMaxPointsFileSize() + getMaxPointsAvgSpeed() + getMaxPointsPriority() + getMaxPointsPercentageOwned();
}

int Engine::getMaxPointsFileSize() const {
  return maxpointsfilesize;
}

int Engine::getMaxPointsAvgSpeed() const {
  return maxpointsavgspeed;
}

int Engine::getMaxPointsPriority() const {
  return maxpointspriority;
}

int Engine::getMaxPointsPercentageOwned() const {
  return maxpointspercentageowned;
}

int Engine::getMaxPointsLowProgress() const {
  return maxpointslowprogress;
}

int Engine::getPriorityPoints(int priority) const {
  switch (priority) {
    case SITE_PRIORITY_VERY_LOW:
      return 0;
    case SITE_PRIORITY_LOW:
      return maxpointspriority * 0.2;
    case SITE_PRIORITY_NORMAL:
      return maxpointspriority * 0.4;
    case SITE_PRIORITY_HIGH:
      return maxpointspriority * 0.6;
    case SITE_PRIORITY_VERY_HIGH:
      return maxpointspriority;
  }
  return 0;
}

int Engine::getSpeedPoints(int avgspeed) const {
  if (maxavgspeed) {
    unsigned long long int pointsavgspeed = maxpointsavgspeed;
    pointsavgspeed *= avgspeed;
    pointsavgspeed /= maxavgspeed;
    return pointsavgspeed;
  }
  return 0;
}

int Engine::getPreparedRaceExpiryTime() const {
  return preparedraceexpirytime;
}

void Engine::setPreparedRaceExpiryTime(int expirytime) {
  preparedraceexpirytime = expirytime;
}

void Engine::clearSkipListCaches() {
  for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
    (*it)->getSkipList().wipeCache();
  }
}
