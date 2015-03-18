#include "filelist.h"

#include <list>

#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "timereference.h"

extern GlobalContext * global;

FileList::FileList(std::string username, std::string path) {
  this->username = username;
  bool endswithslash = path.rfind("/") + 1 == path.length();
  if (endswithslash && path.length() > 1) {
    path = path.substr(0, path.length() - 1);
  }
  this->path = path;
  filled = false;
  owned = 0;
  ownpercentage = 0;
  maxfilesize = 0;
  uploadedfiles = 0;
  locked = false;
  listchanged = false;
  lastchangedstamp = 0;
}

FileList::~FileList() {
  for (std::map<std::string, File *>::iterator it = files.begin(); it != files.end(); it++) {
    delete it->second;
  }
}

bool FileList::updateFile(std::string start, int touch) {
  File * file = new File(start, touch);
  std::string name = file->getName();
  if (name == "." || name == "..") {
    delete file;
    return false;
  }
  File * updatefile;
  if ((updatefile = getFile(name)) != NULL) {
    if (updatefile->getSize() == 0 && file->getSize() > 0) uploadedfiles++;
    if (updatefile->setSize(file->getSize())) {
      setChanged();
    }
    if (file->getSize() > maxfilesize) maxfilesize = file->getSize();
    if (updatefile->getOwner().compare(file->getOwner()) != 0) {
      if (file->getOwner().compare(username) == 0) {
        editOwnedFileCount(true);
      }
      else if (updatefile->getOwner().compare(username) == 0) {
        editOwnedFileCount(false);
      }
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
      if (updatefile->getOwner().compare(username) == 0) {
        updatefile->getUpdateSrc()->pushTransferSpeed(updatefile->getUpdateDst(), updatefile->getUpdateSpeed());
        //std::cout << "pushed new transfer speed..." << std::endl;
      }
      updatefile->unsetUpdateFlag();
    }
    delete file;
    return true;
  }
  else {
    files[name] = file;
    if (file->getSize() > 0) uploadedfiles++;
    if (file->getSize() > maxfilesize) maxfilesize = file->getSize();
    if (file->getOwner().compare(username) == 0) {
      editOwnedFileCount(true);
    }
    setChanged();
  }
  return true;
}

void FileList::touchFile(std::string name, std::string user) {
  touchFile(name, user, false);
}

void FileList::touchFile(std::string name, std::string user, bool upload) {
  File * file;
  if ((file = getFile(name)) != NULL) {
    file->unsetUpdateFlag();
  }
  else {
    file = new File(name, user);
    files[name] = file;
    if (user.compare(username) == 0) {
      editOwnedFileCount(true);
    }
    setChanged();
  }
  if (upload) {
    file->upload();
  }
}

void FileList::setFileUpdateFlag(std::string name, long int size, unsigned int speed, Site * src, std::string dst) {
  File * file;
  if ((file = getFile(name)) != NULL) {
    if (file->setSize(size)) {
      setChanged();
    }
    file->setUpdateFlag(src, dst, speed);
  }
}

File * FileList::getFile(std::string name) const {
  std::map<std::string, File *>::const_iterator it = files.find(name);
  if (it == files.end()) return NULL;
  else return (*it).second;
}

bool FileList::isFilled() const {
  return filled;
}

void FileList::setFilled() {
  filled = true;
}

std::map<std::string, File *>::iterator FileList::begin() {
  return files.begin();
}

std::map<std::string, File *>::iterator FileList::end() {
  return files.end();
}

std::map<std::string, File *>::const_iterator FileList::begin() const {
  return files.begin();
}

std::map<std::string, File *>::const_iterator FileList::end() const {
  return files.end();
}

bool FileList::contains(std::string name) const {
  bool ret = false;
  if (files.find(name) != files.end()) ret = true;
  return ret;
}

unsigned int FileList::getSize() const {
  return files.size();
}

unsigned int FileList::getNumUploadedFiles() const {
  unsigned int count = 0;
  std::map<std::string, File *>::const_iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    if (!it->second->isDirectory() && it->second->getSize() > 0) {
      ++count;
    }
  }
  return count;
}

int FileList::getSizeUploaded() const {
  return uploadedfiles;
}

bool FileList::hasSFV() const {
  std::map<std::string, File *>::const_iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    if(it->second->getExtension() == ("sfv") &&
        it->second->getSize() > 0) {
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

std::string FileList::getPath() const {
  return path;
}

void FileList::cleanSweep(int touch) {
  std::list<std::string> eraselist;
  std::map<std::string, File *>::iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    File * f = it->second;
    if (f->getTouch() != touch && !f->isUploading()) {
      if (f->getOwner().compare(username) == 0) {
        editOwnedFileCount(false);
      }
      if (f->getSize() > 0) uploadedfiles--;
      eraselist.push_back(it->first);
      delete f;
    }
  }
  if (eraselist.size() > 0) {
    for (std::list<std::string>::iterator it2 = eraselist.begin(); it2 != eraselist.end(); it2++) {
      files.erase(*it2);
    }
    maxfilesize = 0;
    for (it = files.begin(); it != files.end(); it++) {
      if (it->second->getSize() > maxfilesize) maxfilesize = it->second->getSize();
    }
    setChanged();
  }
}

void FileList::flush() {
  std::map<std::string, File *>::iterator it;
  for (it = files.begin(); it != files.end(); it++) {
    delete (*it).second;
  }
  files.clear();
  maxfilesize = 0;
  uploadedfiles = 0;  
  owned = 0;
  ownpercentage = 0;
}

void FileList::editOwnedFileCount(bool add) {
 if (add) ++owned;
 else --owned;
 ownpercentage = (owned * 100) / files.size();
}

void FileList::uploadFail(std::string file) {
  uploadattempts[file] = MAXTRANSFERATTEMPTS;
}

void FileList::downloadFail(std::string file) {
  downloadattempts[file] = MAXTRANSFERATTEMPTS;
}

void FileList::addUploadAttempt(std::string file) {
  if (uploadattempts.find(file) == uploadattempts.end()) {
    uploadattempts[file] = 0;
  }
  uploadattempts[file]++;
}

void FileList::downloadAttemptFail(std::string file) {
  if (downloadattempts.find(file) == downloadattempts.end()) {
    downloadattempts[file] = 0;
  }
  downloadattempts[file]++;
}

bool FileList::hasFailedDownload(std::string file) const {
  std::map<std::string, int>::const_iterator it = downloadattempts.find(file);
  if (it == downloadattempts.end()) {
    return false;
  }
  if (it->second < MAXTRANSFERATTEMPTS) {
    return false;
  }
  return true;
}

bool FileList::hasFailedUpload(std::string file) const {
  std::map<std::string, int>::const_iterator it = uploadattempts.find(file);
  if (it == uploadattempts.end()) {
    return false;
  }
  if (it->second < MAXTRANSFERATTEMPTS) {
    return false;
  }
  return true;
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
}

unsigned long long FileList::timeSinceLastChanged() {
  return global->getTimeReference()->timePassedSince(lastchangedstamp);
}

std::string FileList::getUser() const {
  return username;
}
