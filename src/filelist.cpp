#include "filelist.h"

FileList::FileList(std::string username, std::string path) {
  pthread_mutex_init(&filelist_mutex, NULL);
  pthread_mutex_init(&owned_mutex, NULL);
  pthread_mutex_init(&filled_mutex, NULL);
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
  locked = false;
}

bool FileList::updateFile(std::string start, int touch) {
  File * file = new File(start, touch);
  std::string name = file->getName();
  File * updatefile;
  pthread_mutex_lock(&filelist_mutex);
  if ((updatefile = getFileIntern(name)) != NULL) {
    if (updatefile->getSize() == 0 && file->getSize() > 0) uploadedfiles++;
    updatefile->setSize(file->getSize());
    if (file->getSize() > maxfilesize) maxfilesize = file->getSize();
    if (updatefile->getOwner().compare(file->getOwner()) != 0) {
      if (file->getOwner().compare(username) == 0) {
        editOwnedFileCount(true);
      }
      else if (updatefile->getOwner().compare(username) == 0) {
        editOwnedFileCount(false);
      }
    }
    updatefile->setOwner(file->getOwner());
    updatefile->setGroup(file->getGroup());
    updatefile->setLastModified(file->getLastModified());
    updatefile->setTouch(file->getTouch());
    if (updatefile->updateFlagSet()) {
      if (updatefile->getOwner().compare(username) == 0) {
        updatefile->getUpdateSrc()->pushTransferSpeed(updatefile->getUpdateDst(), updatefile->getUpdateSpeed());
        //std::cout << "pushed new transfer speed..." << std::endl;
      }
      updatefile->unsetUpdateFlag();
    }
    pthread_mutex_unlock(&filelist_mutex);
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
  }
  pthread_mutex_unlock(&filelist_mutex);
  return true;
}

void FileList::touchFile(std::string name, std::string user) {
  File * file;
  pthread_mutex_lock(&filelist_mutex);
  if ((file = getFileIntern(name)) != NULL) {
    file->unsetUpdateFlag();
  }
  else {
    file = new File(name, user);
    files[name] = file;
  }
  pthread_mutex_unlock(&filelist_mutex);
}

void FileList::setFileUpdateFlag(std::string name, unsigned int speed, Site * src, std::string dst) {
  File * file;
  pthread_mutex_lock(&filelist_mutex);
  if ((file = getFileIntern(name)) != NULL) {
    file->setUpdateFlag(src, dst, speed);
  }
  pthread_mutex_unlock(&filelist_mutex);
}

File * FileList::getFile(std::string name) {
  return getFile(name, true);
}

File * FileList::getFileIntern(std::string name) {
  return getFile(name, false);
}

File * FileList::getFile(std::string name, bool lock) {
  if (lock) pthread_mutex_lock(&filelist_mutex);
  std::map<std::string, File *>::iterator it = files.find(name);
  if (lock) pthread_mutex_unlock(&filelist_mutex);
  if (it == files.end()) return NULL;
  else return (*it).second;
}

bool FileList::isFilled() {
  bool ret;
  pthread_mutex_lock(&filled_mutex);
  ret = filled;
  pthread_mutex_unlock(&filled_mutex);
  return ret;
}

void FileList::setFilled() {
  pthread_mutex_lock(&filled_mutex);
  filled = true;
  pthread_mutex_unlock(&filled_mutex);
}

std::map<std::string, File *>::iterator FileList::begin() {
  return files.begin();
}

std::map<std::string, File *>::iterator FileList::end() {
  return files.end();
}

bool FileList::contains(std::string name) {
  pthread_mutex_lock(&filelist_mutex);
  bool ret = false;
  if (files.find(name) != files.end()) ret = true;
  pthread_mutex_unlock(&filelist_mutex);
  return ret;
}

int FileList::getSize() {
  int ret;
  if (!locked) pthread_mutex_lock(&filelist_mutex);
  ret = files.size();
  if (!locked) pthread_mutex_unlock(&filelist_mutex);
  return ret;
}

int FileList::getNumUploadedFiles() {
  int count = 0;
  std::map<std::string, File *>::iterator it;
  pthread_mutex_lock(&filelist_mutex);
  for (it = files.begin(); it != files.end(); it++) {
    if (!it->second->isDirectory() && it->second->getSize() > 0) {
      ++count;
    }
  }
  pthread_mutex_unlock(&filelist_mutex);
  return count;
}

int FileList::getSizeUploaded() {
  int ret;
  pthread_mutex_lock(&filelist_mutex);
  ret = uploadedfiles;
  pthread_mutex_unlock(&filelist_mutex);
  return ret;
}

bool FileList::hasSFV() {
  std::map<std::string, File *>::iterator it;
  pthread_mutex_lock(&filelist_mutex);
  for (it = files.begin(); it != files.end(); it++) {
    if(it->second->getExtension() == ("sfv") &&
        it->second->getSize() > 0) {
      pthread_mutex_unlock(&filelist_mutex);
      return true;
    }
  }
  pthread_mutex_unlock(&filelist_mutex);
  return false;
}

int FileList::getOwnedPercentage() {
  int ret;
  pthread_mutex_lock(&owned_mutex);
  ret = ownpercentage;
  pthread_mutex_unlock(&owned_mutex);
  return ret;
}

unsigned long long int FileList::getMaxFileSize() {
  return maxfilesize;
}

std::string FileList::getPath() {
  return path;
}

void FileList::lockFileList() {
  pthread_mutex_lock(&filelist_mutex);
  locked = true;
}

void FileList::unlockFileList() {
  locked = false;
  pthread_mutex_unlock(&filelist_mutex);
}

void FileList::cleanSweep(int touch) {
  std::map<std::string, File *>::iterator it;
  pthread_mutex_lock(&filelist_mutex);
  for (it = files.begin(); it != files.end(); it++) {
    File * f = it->second;
    if (f->getTouch() != touch) {
      if (f->getOwner().compare(username) == 0) {
        editOwnedFileCount(false);
      }
      files.erase(it);
      if (f->getSize() > 0) uploadedfiles--;
      if (f->getSize() == maxfilesize) {
        maxfilesize = 0;
        std::map<std::string, File *>::iterator it2;
        for (it2 = files.begin(); it2 != files.end(); it2++) {
          if (it2->second->getSize() > maxfilesize) maxfilesize = it2->second->getSize();
        }
      }
      delete f;
    }
  }
  pthread_mutex_unlock(&filelist_mutex);
}

void FileList::flush() {
  std::map<std::string, File *>::iterator it;
  pthread_mutex_lock(&filelist_mutex);
  for (it = files.begin(); it != files.end(); it++) {
    delete (*it).second;
  }
  files.clear();
  maxfilesize = 0;
  uploadedfiles = 0;  
  pthread_mutex_lock(&owned_mutex);
  owned = 0;
  ownpercentage = 0;
  pthread_mutex_unlock(&owned_mutex);
  pthread_mutex_unlock(&filelist_mutex);
}

void FileList::editOwnedFileCount(bool add) {
 pthread_mutex_lock(&owned_mutex);
 if (add) ++owned;
 else --owned;
 ownpercentage = (owned * 100) / files.size();
 pthread_mutex_unlock(&owned_mutex);
}
