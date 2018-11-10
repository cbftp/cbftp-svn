#include "filelist.h"

#include <list>

#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "timereference.h"
#include "util.h"
#include "commandowner.h"

#define MAX_LIST_FAILURES_BEFORE_STATE_FAILED 3

namespace {

bool sameOwner(const std::string & a, const std::string & b) {
  size_t alen = a.length();
  if (alen > 8) {
    size_t blen = b.length();
    if (alen < blen) {
      return a.compare(0, alen, b, 0, alen) == 0;
    }
    return a.compare(0, blen, b, 0, blen) == 0;
  }
  return a.compare(0, 8, b, 0, 8) == 0;
}


}

FileList::FileList(const std::string & username, const Path & path) {
  init(username, path, FILELIST_UNKNOWN);
}

FileList::FileList(const std::string & username, const Path & path, FileListState state) {
  init(username, path, state);
}

FileList::~FileList() {
  for (std::unordered_map<std::string, File *>::iterator it = files.begin(); it != files.end(); it++) {
    delete it->second;
  }
}

void FileList::init(const std::string & username, const Path & path, FileListState state) {
  this->username = username;
  this->path = path;
  this->state = state;
  owned = 0;
  ownpercentage = 100;
  maxfilesize = 0;
  uploadedfiles = 0;
  locked = false;
  listchanged = false;
  lastchangedstamp = 0;
  totalfilesize = 0;
  uploading = 0;
  refreshedtime = 0;
  listfailures = 0;
}

bool FileList::updateFile(const std::string & start, int touch) {
  File * file = new File(start, touch);
  if (!file->isValid()) {
    delete file;
    return false;
  }
  std::string name = file->getName();
  if (name == "." || name == "..") {
    delete file;
    return false;
  }
  File * updatefile;
  if ((updatefile = getFileCaseSensitive(name)) != NULL) {
    if (updatefile->getSize() == 0 && file->getSize() > 0 && !file->isDirectory()) uploadedfiles++;
    unsigned long long int oldsize = updatefile->getSize();
    unsigned long long int newsize = file->getSize();
    if (updatefile->setSize(newsize)) {
      if (!updatefile->isDirectory()) {
        totalfilesize += newsize - oldsize;
      }
      setChanged();
    }
    if (newsize > maxfilesize) maxfilesize = newsize;
    if (!sameOwner(file->getOwner(), updatefile->getOwner())) {
      if (sameOwner(username, file->getOwner())) {
        owned++;
      }
      else if (sameOwner(username, updatefile->getOwner()))
      {
        owned--;
      }
      recalcOwnedPercentage();
    }
    if (updatefile->setOwner(file->getOwner())) {
      setChanged();
    }
    if (updatefile->setGroup(file->getGroup())) {
      setChanged();
    }
    if (updatefile->setLastModified(file->getLastModified())) {
      setChanged();
    }
    updatefile->setTouch(file->getTouch());
    if (updatefile->updateFlagSet()) {
      unsigned long long int size = oldsize;
      unsigned int speed = updatefile->getUpdateSpeed();
      const std::shared_ptr<Site> & src = updatefile->getUpdateSrc();
      const std::shared_ptr<Site> & dst = updatefile->getUpdateDst();
      if (sameOwner(username, updatefile->getOwner())) {
        size = newsize;
        std::string extension = updatefile->getExtension();
        if (extension != "sfv" && extension != "nfo") {
          updatefile->getUpdateSrc()->pushTransferSpeed(dst->getName(), speed, size);
        }
      }
      src->addTransferStatsFile(STATS_DOWN, dst->getName(), size);
      dst->addTransferStatsFile(STATS_UP, src->getName(), size);
      updatefile->getUpdateSrcCommandOwner()->addTransferStatsFile(STATS_DOWN, dst->getName(), size, speed);
      updatefile->getUpdateDstCommandOwner()->addTransferStatsFile(STATS_UP, src->getName(), size, speed);
      global->getStatistics()->addTransferStatsFile(STATS_FXP, size);
      updatefile->unsetUpdateFlag();
    }
    delete file;
  }
  else {
    files[name] = file;
    lowercasefilemap[util::toLower(name)] = name;
    unsigned long long int filesize = file->getSize();
    if (filesize > 0 && !file->isDirectory()) {
      uploadedfiles++;
      totalfilesize += filesize;
    }
    if (filesize > maxfilesize) maxfilesize = filesize;
    if (sameOwner(username, file->getOwner())) {
      owned++;
    }
    recalcOwnedPercentage();
    setChanged();
  }
  return true;
}

void FileList::touchFile(const std::string & name, const std::string & user) {
  touchFile(name, user, false);
}

void FileList::touchFile(const std::string & name, const std::string & user, bool upload) {
  File * file;
  if ((file = getFileCaseSensitive(name)) != NULL) {
    file->unsetUpdateFlag();
  }
  else {
    file = new File(name, user);
    files[name] = file;
    lowercasefilemap[util::toLower(name)] = name;
    if (sameOwner(username, user)) {
      owned++;
    }
    recalcOwnedPercentage();
    setChanged();
  }
  if (upload && !file->isUploading()) {
    file->upload();
    uploading++;
  }
}

void FileList::removeFile(const std::string & name) {
  File * f;
  if ((f = getFileCaseSensitive(name)) != NULL) {
    if (sameOwner(username, f->getOwner())) {
      owned--;
    }
    if (f->getSize() > 0 && !f->isDirectory()) {
      uploadedfiles--;
      totalfilesize -= f->getSize();
    }
    if (f->isUploading()) {
      uploading--;
    }
    delete f;
    files.erase(name);
    lowercasefilemap.erase(util::toLower(name));
    recalcOwnedPercentage();
    setChanged();
  }
}

void FileList::setFileUpdateFlag(const std::string & name,
    unsigned long long int size, unsigned int speed, const std::shared_ptr<Site> & src,
    const std::shared_ptr<Site> & dst, CommandOwner * srcco, CommandOwner * dstco)
{
  File * file;
  if ((file = getFileCaseSensitive(name)) != NULL) {
    unsigned long long int oldsize = file->getSize();
    if (file->setSize(size)) {
      if (!oldsize) {
        uploadedfiles++;
      }
      totalfilesize += size - oldsize;
      if (size > maxfilesize) {
        maxfilesize = size;
      }
      setChanged();
    }
    file->setUpdateFlag(src, dst, srcco, dstco, speed);
  }
}

File * FileList::getFile(const std::string & name) const {
  std::unordered_map<std::string, File *>::const_iterator it = files.find(name);
  if (it == files.end()) {
    std::unordered_map<std::string, std::string>::const_iterator it2 = lowercasefilemap.find(util::toLower(name));
    if (it2 != lowercasefilemap.end()) {
      it = files.find(it2->second);
    }
    if (it == files.end()) {
      return NULL;
    }
  }
  return it->second;
}

File * FileList::getFileCaseSensitive(const std::string & name) const {
  std::unordered_map<std::string, File *>::const_iterator it = files.find(name);
  if (it == files.end()) {
    return NULL;
  }
  return it->second;
}

FileListState FileList::getState() const {
  return state;
}

void FileList::setNonExistent() {
  util::assert(state == FILELIST_UNKNOWN);
  state = FILELIST_NONEXISTENT;
}

void FileList::setExists() {
  state = FILELIST_EXISTS;
}

void FileList::setFilled() {
  state = FILELIST_LISTED;
}

void FileList::setFailed() {
  state = FILELIST_FAILED;
}

std::unordered_map<std::string, File *>::iterator FileList::begin() {
  return files.begin();
}

std::unordered_map<std::string, File *>::iterator FileList::end() {
  return files.end();
}

std::unordered_map<std::string, File *>::const_iterator FileList::begin() const {
  return files.begin();
}

std::unordered_map<std::string, File *>::const_iterator FileList::end() const {
  return files.end();
}

bool FileList::contains(const std::string & name) const {
  if (files.find(name) != files.end()) {
    return true;
  }
  std::unordered_map<std::string, std::string>::const_iterator it = lowercasefilemap.find(util::toLower(name));
  if (it != lowercasefilemap.end() && files.find(it->second) != files.end()) {
    return true;
  }
  return false;
}

bool FileList::containsPattern(const std::string & pattern, bool dir) const {
  size_t slashpos = pattern.rfind('/');
  const std::string * usedpattern = &pattern;
  std::string newpattern;
  if (slashpos != std::string::npos) {
    newpattern = pattern.substr(slashpos + 1);
    usedpattern = &newpattern;
  }
  for (std::unordered_map<std::string, File *>::const_iterator it = files.begin(); it != files.end(); it++) {
    if (it->second->isDirectory() == dir && util::wildcmp(usedpattern->c_str(), it->first.c_str())) {
      return true;
    }
  }
  return false;
}

bool FileList::containsPatternBefore(const std::string & pattern, bool dir, const std::string & item) const {
  size_t slashpos = pattern.rfind('/');
  const std::string * usedpattern = &pattern;
  std::string newpattern;
  if (slashpos != std::string::npos) {
    newpattern = pattern.substr(slashpos + 1);
    usedpattern = &newpattern;
  }
  for (std::unordered_map<std::string, File *>::const_iterator it = files.begin(); it != files.end(); it++) {
    if (it->second->isDirectory() != dir) {
      continue;
    }
    if (it->first == item) {
      return false;
    }
    if (util::wildcmp(usedpattern->c_str(), it->first.c_str())) {
      return true;
    }
  }
  return false;
}

unsigned int FileList::getSize() const {
  return files.size();
}

unsigned long long int FileList::getTotalFileSize() const {
  return totalfilesize;
}

unsigned int FileList::getNumUploadedFiles() const {
  return uploadedfiles;
}

bool FileList::hasSFV() const {
  std::unordered_map<std::string, File *>::const_iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    if(it->second->getExtension() == "sfv" && it->second->getSize() > 0) {
      return true;
    }
  }
  return false;
}

int FileList::getOwnedPercentage() const {
  return ownpercentage;
}

unsigned long long int FileList::getMaxFileSize() const {
  return maxfilesize;
}

const Path & FileList::getPath() const {
  return path;
}

void FileList::cleanSweep(int touch) {
  std::list<std::string> eraselist;
  std::unordered_map<std::string, File *>::iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    File * f = it->second;
    if (f->getTouch() != touch && !f->isUploading()) {
      eraselist.push_back(it->first);
    }
  }
  if (eraselist.size() > 0) {
    for (std::list<std::string>::iterator it2 = eraselist.begin(); it2 != eraselist.end(); it2++) {
      removeFile(*it2);
      files.erase(*it2);
      lowercasefilemap.erase(util::toLower(*it2));
    }
    maxfilesize = 0;
    for (it = files.begin(); it != files.end(); it++) {
      if (it->second->getSize() > maxfilesize) maxfilesize = it->second->getSize();
    }
    setChanged();
  }
}

void FileList::flush() {
  std::unordered_map<std::string, File *>::iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    delete (*it).second;
  }
  files.clear();
  lowercasefilemap.clear();
  maxfilesize = 0;
  totalfilesize = 0;
  uploadedfiles = 0;  
  owned = 0;
  ownpercentage = 100;
  uploading = 0;
}

void FileList::recalcOwnedPercentage() {
 if (!files.empty()) {
   ownpercentage = (owned * 100) / files.size();
 }
 else {
   ownpercentage = 100;
 }
}

bool FileList::listChanged() const {
  return listchanged;
}

void FileList::resetListChanged() {
  listchanged = false;
}

void FileList::setChanged() {
  listchanged = true;
  lastchangedstamp = global->getTimeReference()->timeReference();
  listfailures = 0;
}

unsigned long long FileList::timeSinceLastChanged() const {
  return global->getTimeReference()->timePassedSince(lastchangedstamp);
}

std::string FileList::getUser() const {
  return username;
}

void FileList::finishUpload(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && fileobj->isUploading()) {
    fileobj->finishUpload();
    uploading--;
    setChanged();
  }
}

void FileList::finishDownload(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && fileobj->isDownloading()) {
    fileobj->finishDownload();
  }
}

void FileList::download(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && !fileobj->isDownloading()) {
    fileobj->download();
  }
}

bool FileList::hasFilesUploading() const {
  return uploading > 0;
}

void FileList::setRefreshedTime(int newtime) {
  refreshedtime = newtime;
}

int FileList::getRefreshedTime() const {
  return refreshedtime;
}

bool FileList::addListFailure() {
  if (++listfailures >= MAX_LIST_FAILURES_BEFORE_STATE_FAILED) {
    setFailed();
    return true;
  }
  return false;
}
