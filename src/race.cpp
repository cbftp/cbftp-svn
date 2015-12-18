#include "race.h"

#include <vector>
#include <algorithm>

#include "file.h"
#include "filelist.h"
#include "site.h"
#include "siterace.h"
#include "util.h"
#include "globalcontext.h"
#include "tickpoke.h"
#include "transferstatus.h"

extern GlobalContext * global;

Race::Race(unsigned int id, SpreadProfile profile, std::string release, std::string section) :
  name(release),
  group(util::getGroupNameFromRelease(release)),
  section(section),
  maxnumfilessiteprogress(0),
  bestunknownfilesizeestimate(50000000),
  guessedtotalfilesize(0),
  checkcount(0),
  timestamp(util::ctimeLog()),
  timespent(0),
  status(RACE_STATUS_RUNNING),
  worst(0),
  avg(0),
  best(0),
  transferattemptscleared(false),
  id(id),
  profile(profile)
{
  estimatedsubpaths.push_back("");
  guessedfilelists[""] = std::map<std::string, unsigned long long int>();
  setUndone();
}

Race::~Race() {
  if (!isDone()) {
    setDone();
  }
}

void Race::addSite(SiteRace * siterace, SiteLogic * sitelogic) {
  sites.push_back(std::pair<SiteRace *, SiteLogic *>(siterace, sitelogic));
}

void Race::removeSite(SiteLogic * sitelogic) {
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->second == sitelogic) {
      removeSite(it->first);
      break;
    }
  }
}

void Race::removeSite(SiteRace * siterace) {
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->first == siterace) {
      sites.erase(it);
      break;
    }
  }
  for (std::list<SiteRace *>::iterator it = semidonesites.begin(); it != semidonesites.end(); it++) {
    if (*it == siterace) {
      donesites.erase(it);
      break;
    }
  }
  for (std::list<SiteRace *>::iterator it = donesites.begin(); it != donesites.end(); it++) {
    if (*it == siterace) {
      donesites.erase(it);
      break;
    }
  }
  sizes.erase(siterace);
}

std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator Race::begin() const {
  return sites.begin();
}

std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator Race::end() const {
  return sites.end();
}

std::string Race::getName() const {
  return name;
}

std::string Race::getGroup() const {
  return group;
}

std::string Race::getSection() const {
  return section;
}

int Race::numSitesDone() const {
  return donesites.size();
}

int Race::numSites() const {
  return sites.size();
}

bool Race::sizeEstimated(std::string subpath) const {
  return estimatedsize.find(subpath) != estimatedsize.end();
}

unsigned int Race::estimatedSize(std::string subpath) const {
  std::map<std::string, unsigned int>::const_iterator it = estimatedsize.find(subpath);
  if (it != estimatedsize.end()) {
    return it->second;
  }
  return 0;
}

unsigned int Race::guessedSize(std::string subpath) const {
  std::map<std::string, std::map<std::string, unsigned long long int> >::const_iterator it =
      guessedfilelists.find(subpath);
  if (it != guessedfilelists.end()) {
    return it->second.size();
  }
  return 10;
}

unsigned long long int Race::estimatedTotalSize() const {
  return guessedtotalfilesize;
}

unsigned long long int Race::guessedFileSize(std::string subpath, std::string file) const {
  std::map<std::string, std::map<std::string, SizeLocationTrack> >::const_iterator it =
      sizelocationtrackers.find(subpath);
  if (it == sizelocationtrackers.end()) {
    return bestunknownfilesizeestimate;
  }
  std::map<std::string, SizeLocationTrack>::const_iterator it2 = it->second.find(file);
  if (it2 == it->second.end()) {
    return bestunknownfilesizeestimate;
  }
  unsigned long long int thisestimatedsize = it2->second.getEstimatedSize();
  if (!thisestimatedsize) {
    return bestunknownfilesizeestimate;
  }
  return thisestimatedsize;
}

std::map<std::string, unsigned long long int>::const_iterator Race::guessedFileListBegin(std::string subpath) const {
  std::map<std::string, std::map<std::string, unsigned long long int> >::const_iterator it;
  if ((it = guessedfilelists.find(subpath)) != guessedfilelists.end()) {
    return it->second.begin();
  }
  return guessedfilelists.at("").end();
}

std::map<std::string, unsigned long long int>::const_iterator Race::guessedFileListEnd(std::string subpath) const {
  std::map<std::string, std::map<std::string, unsigned long long int> >::const_iterator it;
  if ((it = guessedfilelists.find(subpath)) != guessedfilelists.end()) {
    return it->second.end();
  }
  return guessedfilelists.at("").end();
}

bool Race::SFVReported(std::string subpath) const {
  std::map<std::string, std::list<SiteRace *> >::const_iterator it = sfvreports.find(subpath);
  return it != sfvreports.end();
}

std::list<std::string> Race::getSubPaths() const {
  return estimatedsubpaths;
}

void Race::updateSiteProgress(unsigned int numuploadedfiles) {
  if (maxnumfilessiteprogress < numuploadedfiles) maxnumfilessiteprogress = numuploadedfiles;
}

unsigned int Race::getMaxSiteNumFilesProgress() const {
  return maxnumfilessiteprogress;
}

bool Race::isDone() const {
  return status != RACE_STATUS_RUNNING;
}

void Race::reportNewSubDir(SiteRace * sr, std::string subdir) {
  if (subpathoccurences.find(subdir) == subpathoccurences.end()) {
    subpathoccurences[subdir] = std::list<SiteRace *>();
  }
  std::list<SiteRace *>::iterator it;
  for (it = subpathoccurences[subdir].begin(); it != subpathoccurences[subdir].end(); it++) {
    if (*it == sr) {
      return;
    }
  }
  subpathoccurences[subdir].push_back(sr);
  if (subpathoccurences[subdir].size() >= sites.size() * 0.5) {
    std::list<std::string>::iterator it2;
    for (it2 = estimatedsubpaths.begin(); it2 != estimatedsubpaths.end(); it2++) {
      if (*it2 == subdir) {
        return;
      }
    }
    estimatedsubpaths.push_back(subdir);
    guessedfilelists[subdir] = std::map<std::string, unsigned long long int>();
  }
}

void Race::reportSFV(SiteRace * sr, std::string subpath) {
  std::map<std::string, std::list<SiteRace *> >::iterator it = sfvreports.find(subpath);
  if (it == sfvreports.end()) {
    sfvreports[subpath] = std::list<SiteRace *>();
  }
  for (std::list<SiteRace *>::iterator it2 = sfvreports[subpath].begin(); it2 != sfvreports[subpath].end(); it2++) {
    if (*it2 == sr) {
      return;
    }
  }
  sfvreports[subpath].push_back(sr);
}

void Race::reportDone(SiteRace * sr) {
  std::list<SiteRace *>::iterator it;
  for (it = donesites.begin(); it != donesites.end(); it++) {
    if (*it == sr) {
      return;
    }
  }
  reportSemiDone(sr);
  donesites.push_back(sr);
  if (donesites.size() == sites.size()) {
    setDone();
  }
}

void Race::reportSemiDone(SiteRace * sr) {
  std::list<SiteRace *>::iterator it;
  for (it = semidonesites.begin(); it != semidonesites.end(); it++) {
    if (*it == sr) {
      return;
    }
  }
  semidonesites.push_back(sr);
  if (semidonesites.size() == sites.size()) {
    setDone();
    for (it = semidonesites.begin(); it != semidonesites.end(); it++) {
      (*it)->complete(false);
    }
  }
}

void Race::setUndone() {
  status = RACE_STATUS_RUNNING;
  clearTransferAttempts();
  resetUpdateCheckCounter();
  global->getTickPoke()->startPoke(this, "Race", RACE_UPDATE_INTERVAL, 0);
}

void Race::reset() {
  clearTransferAttempts();
  resetUpdateCheckCounter();
  if (isDone()) {
    setUndone();
  }
  semidonesites.clear();
  donesites.clear();
  sizes.clear();
  estimatedsubpaths.clear();
  estimatedsubpaths.push_back("");
  guessedfilelists.clear();
  guessedfilelists[""] = std::map<std::string, unsigned long long int>();
  sfvreports.clear();
  estimatedsize.clear();
  estimatedfilesizes.clear();
  subpathoccurences.clear();
  guessedfileliststotalfilesize.clear();
  sizelocationtrackers.clear();
  maxnumfilessiteprogress = 0;
  worst = 0;
  avg = 0;
  best = 0;
  guessedtotalfilesize = 0;
  bestunknownfilesizeestimate = 50000000;
}

void Race::abort() {
  setDone();
  status = RACE_STATUS_ABORTED;
}

void Race::setTimeout() {
  setDone();
  status = RACE_STATUS_TIMEOUT;
}

void Race::reportSize(SiteRace * sr, FileList * fl, std::string subpath, std::list<std::string > * uniques, bool final) {
  std::list<std::string>::iterator itu;
  File * file;
  if (sizelocationtrackers.find(subpath) == sizelocationtrackers.end()) {
    sizelocationtrackers[subpath] = std::map<std::string, SizeLocationTrack>();
  }
  bool recalc = false;
  for (itu = uniques->begin(); itu != uniques->end(); itu++) {
    std::string unique = *itu;
    if (sizelocationtrackers[subpath].find(unique) == sizelocationtrackers[subpath].end()) {
      sizelocationtrackers[subpath][unique] = SizeLocationTrack();
    }
    if ((file = fl->getFile(unique)) != NULL) {
      if (sizelocationtrackers[subpath][unique].add(sr, file->getSize())) {
        estimatedfilesizes[unique] = sizelocationtrackers[subpath][unique].getEstimatedSize();
        recalc = true;
      }
    }
    else {
      sizelocationtrackers[subpath][unique].add(sr, 0);
    }
  }
  if (recalc) {
    recalculateBestUnknownFileSizeEstimate();
  }
  if (sizes.find(sr) == sizes.end()) {
    sizes[sr] = std::map<std::string, unsigned int>();
  }
  if (guessedfilelists.find(subpath) != guessedfilelists.end()) {
    guessedfilelists[subpath].clear();
    std::map<std::string, SizeLocationTrack>::iterator it;
    int highestnumsites = 0;
    for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
      int thisnumsites = it->second.numSites();
      if (thisnumsites > highestnumsites) {
        highestnumsites = thisnumsites;
      }
    }
    int minnumsites = highestnumsites / 2;
    unsigned long long int aggregatedsize = 0;
    for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
      if (it->second.numSites() > minnumsites || sites.size() == 2) {
        unsigned long long int estimatedsize = it->second.getEstimatedSize();
        guessedfilelists[subpath][it->first] = estimatedsize;
        aggregatedsize += estimatedsize;
      }
    }
    if (guessedfileliststotalfilesize[subpath] != aggregatedsize) {
      guessedfileliststotalfilesize[subpath] = aggregatedsize;
      calculateTotalFileSize();
    }
    if (final) {
      sizes[sr][subpath] = uniques->size();
      std::map<SiteRace *, std::map<std::string, unsigned int> >::iterator it;
      std::map<std::string, unsigned int>::iterator it2;
      std::vector<unsigned int> subpathsizes;
      for (it = sizes.begin(); it != sizes.end(); it++) {
        it2 = it->second.find(subpath);
        if (it2 != it->second.end()) {
          subpathsizes.push_back(it2->second);
        }
      }
      // stupid formula, replace with time check from race start
      if (subpathsizes.size() == sites.size() ||
        (subpathsizes.size() >= sites.size() * 0.8 && sites.size() > 2)) {
        estimatedsize[subpath] = guessedfilelists[subpath].size();
      }
    }
  }
}

void Race::recalculateBestUnknownFileSizeEstimate() {
  std::map<std::string, unsigned long long int>::iterator it;
  unsigned long long int size;
  std::map<unsigned long long int, int> commonsizes;
  std::map<unsigned long long int, int>::iterator it2;
  for (it = estimatedfilesizes.begin(); it != estimatedfilesizes.end(); it++) {
    size = it->second;
    if (!size) {
      continue;
    }
    it2 = commonsizes.find(size);
    if (it2 != commonsizes.end()) {
      commonsizes[size] = it2->second + 1;
    }
    else {
      commonsizes[size] = 1;
    }
  }
  unsigned long long int mostcommon = 0;
  int mostcommoncount = 0;
  unsigned long long int largest = 0;
  int largestcount = 1;
  for (it2 = commonsizes.begin(); it2 != commonsizes.end(); it2++) {
    if (it2->second > mostcommoncount) {
      mostcommon = it2->first;
      mostcommoncount = it2->second;
    }
    if (it2->first > largest) {
      largest = it2->first;
      largestcount = 1;
    }
    else if (it2->first == largest) {
      largestcount++;
    }
  }
  if (largestcount >= 2 || largestcount == mostcommoncount ||
      (mostcommon == 0 && mostcommoncount + 1 < (int)commonsizes.size())) {
    bestunknownfilesizeestimate = largest;
  }
  else {
    bestunknownfilesizeestimate = mostcommon;
  }
}

int Race::checksSinceLastUpdate() {
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->first->hasBeenUpdatedSinceLastCheck()) {
      checkcount = 0;
    }
  }
  return checkcount++;
}

void Race::resetUpdateCheckCounter() {
  checkcount = 0;
}

std::string Race::getTimeStamp() const {
  return timestamp;
}

void Race::setDone() {
  status = RACE_STATUS_DONE;
  global->getTickPoke()->stopPoke(this, 0);
}

void Race::tick(int) {
  timespent += RACE_UPDATE_INTERVAL;
  calculatePercentages();
}

void Race::calculatePercentages() {
  unsigned int totalpercentage = 0;
  unsigned int localworst = 100;
  unsigned int localbest = 0;
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = begin(); it != end(); it++) {
    unsigned int percentagecomplete = guessedtotalfilesize
        ? (it->first->getTotalFileSize() * 100) / guessedtotalfilesize
        : 0;
    if (percentagecomplete > 100) {
      percentagecomplete = 100;
    }
    totalpercentage += percentagecomplete;
    if (percentagecomplete < localworst) {
      localworst = percentagecomplete;
    }
    if (percentagecomplete > localbest) {
      localbest = percentagecomplete;
    }
  }
  avg = totalpercentage / sites.size();
  worst = localworst;
  best = localbest;
}

void Race::calculateTotalFileSize() {
  unsigned long long int aggregatedsize = 0;
  for (std::map<std::string, unsigned long long int>::const_iterator it =
      guessedfileliststotalfilesize.begin(); it != guessedfileliststotalfilesize.end(); it++) {
    aggregatedsize += it->second;
  }
  guessedtotalfilesize = aggregatedsize;
}

unsigned int Race::getTimeSpent() const {
  return timespent / 1000;
}

std::string Race::getSiteListText() const {
  std::string sitestr = "";
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = begin(); it != end(); it++) {
    sitestr += it->first->getSiteName() + ",";
  }
  if (sitestr.length() > 0) {
    sitestr = sitestr.substr(0, sitestr.length() - 1);
  }
  return sitestr;
}

int Race::getStatus() const {
  return status;
}

unsigned int Race::getId() const {
  return id;
}

SpreadProfile Race::getProfile() const {
  return profile;
}

SiteRace * Race::getSiteRace(std::string site) const {
  for (std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator it = begin(); it != end(); it++) {
    if (it->first->getSiteName() == site) {
      return it->first;
    }
  }
  return NULL;
}

unsigned int Race::getWorstCompletionPercentage() const {
  return worst;
}

unsigned int Race::getAverageCompletionPercentage() const {
  return avg;
}

unsigned int Race::getBestCompletionPercentage() const {
  return best;
}

bool Race::hasFailedTransfer(File * f, FileList * fld) const {
  std::map<std::pair<File *, FileList *>, int>::const_iterator it =
      transferattempts.find(std::pair<File *, FileList *>(f, fld));
  return it != transferattempts.end() && it->second >= MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP;
}

bool Race::failedTransfersCleared() const {
  return transferattemptscleared;
}

void Race::addTransfer(Pointer<TransferStatus> & ts) {
  ts->setCallback(this);
}

bool Race::clearTransferAttempts() {
  bool ret = transferattempts.size();
  transferattempts.clear();
  transferattemptscleared = true;
  return ret;
}

void Race::transferSuccessful(Pointer<TransferStatus> & ts) {
  addTransferAttempt(ts);
}

void Race::transferFailed(Pointer<TransferStatus> & ts, int) {
  addTransferAttempt(ts);
}

void Race::addTransferAttempt(Pointer<TransferStatus> & ts) {
  File * f = ts->getSourceFileList()->getFile(ts->getFile());
  std::pair<File *, FileList *> matchpair =
      std::pair<File *, FileList *>(f, ts->getTargetFileList());
  std::map<std::pair<File *, FileList *>, int>::iterator it =
      transferattempts.find(matchpair);
  if (it == transferattempts.end()) {
    transferattempts[matchpair] = 1;
  }
  else {
    it->second++;
  }
}
