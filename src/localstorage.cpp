#include "localstorage.h"

#include "localdownload.h"
#include "localupload.h"
#include "transfermonitor.h"
#include "localfilelist.h"
#include "util.h"
#include "eventlog.h"
#include "ftpconn.h"
#include "globalcontext.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unordered_map>

namespace {

BinaryData emptydata;

}

LocalStorage::LocalStorage() :
  temppath("/tmp"),
  downloadpath(std::string(getenv("HOME")) + "/Downloads"),
  storeidcounter(0),
  useactivemodeaddress(false),
  activeportfirst(47500),
  activeportlast(47600),
  currentactiveport(activeportfirst)
{

}

LocalStorage::~LocalStorage() {

}

LocalTransfer * LocalStorage::passiveModeDownload(TransferMonitor * tm, const std::string & file, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
  return passiveModeDownload(tm, temppath, file, host, port, ssl, ftpconn);
}

LocalTransfer * LocalStorage::passiveModeDownload(TransferMonitor * tm, const Path & path, const std::string & file, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
  LocalDownload * ld = getAvailableLocalDownload();
  ld->engage(tm, path, file, host, port, ssl, ftpconn);
  return ld;
}

LocalTransfer * LocalStorage::passiveModeDownload(TransferMonitor * tm, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
  int storeid = storeidcounter++;
  LocalDownload * ld = getAvailableLocalDownload();
  ld->engage(tm, storeid, host, port, ssl, ftpconn);
  return ld;
}

LocalTransfer * LocalStorage::passiveModeUpload(TransferMonitor * tm, const Path & path, const std::string & file, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
  LocalUpload * lu = getAvailableLocalUpload();
  lu->engage(tm, path, file, host, port, ssl, ftpconn);
  return lu;
}

LocalTransfer * LocalStorage::activeModeDownload(TransferMonitor * tm, const Path & path, const std::string & file, bool ssl, FTPConn * ftpconn) {
  int startport = getNextActivePort();
  int port = startport;
  bool entered = false;
  while (!entered || port != startport) {
    entered = true;
    LocalDownload * ld = getAvailableLocalDownload();
    if (ld->engage(tm, path, file, port, ssl, ftpconn)) {
      return ld;
    }
    port = getNextActivePort();
  }
  global->getEventLog()->log("LocalStorage", "Error: no local ports available for active mode");
  return NULL;
}

LocalTransfer * LocalStorage::activeModeDownload(TransferMonitor * tm, bool ssl, FTPConn * ftpconn) {
  int storeid = storeidcounter++;
  int startport = getNextActivePort();
  int port = startport;
  bool entered = false;
  while (!entered || port != startport) {
    entered = true;
    LocalDownload * ld = getAvailableLocalDownload();
    if (ld->engage(tm, storeid, port, ssl, ftpconn)) {
      return ld;
    }
    port = getNextActivePort();
  }
  global->getEventLog()->log("LocalStorage", "Error: no local ports available for active mode");
  return NULL;
}

LocalTransfer * LocalStorage::activeModeUpload(TransferMonitor * tm, const Path & path, const std::string & file, bool ssl, FTPConn * ftpconn) {
  int startport = getNextActivePort();
  int port = startport;
  bool entered = false;
  while (!entered || port != startport) {
    entered = true;
    LocalUpload * lu = getAvailableLocalUpload();
    if (lu->engage(tm, path, file, port, ssl, ftpconn)) {
      return lu;
    }
    port = getNextActivePort();
  }
  global->getEventLog()->log("LocalStorage", "Error: no local ports available for active mode");
  return NULL;
}


LocalDownload * LocalStorage::getAvailableLocalDownload() {
  LocalDownload * ld = NULL;
  for (std::list<LocalDownload *>::iterator it = localdownloads.begin(); it != localdownloads.end(); it++) {
    if (!(*it)->active()) {
      ld = *it;
      break;
    }
  }
  if (ld == NULL) {
    ld = new LocalDownload(this);
    localdownloads.push_back(ld);
  }
  return ld;
}

LocalUpload * LocalStorage::getAvailableLocalUpload() {
  LocalUpload * lu = NULL;
  for (std::list<LocalUpload *>::iterator it = localuploads.begin(); it != localuploads.end(); it++) {
    if (!(*it)->active()) {
      lu = *it;
      break;
    }
  }
  if (lu == NULL) {
    lu = new LocalUpload();
    localuploads.push_back(lu);
  }
  return lu;
}

BinaryData LocalStorage::getTempFileContent(const std::string & filename) const {
  return getFileContent(temppath / filename);
}

BinaryData LocalStorage::getFileContent(const Path & filename) const {
  std::ifstream filestream;
  filestream.open(filename.toString().c_str(), std::ios::binary | std::ios::in);
  char * data = (char *) malloc(MAXREAD);
  filestream.read(data, MAXREAD);
  BinaryData out(data, data + filestream.gcount());
  delete data;
  return out;
}

const BinaryData & LocalStorage::getStoreContent(int storeid) const {
  std::map<int, BinaryData>::const_iterator it = content.find(storeid);
  if (it != content.end()) {
    return it->second;
  }
  return emptydata;
}

void LocalStorage::purgeStoreContent(int storeid) {
  if (content.find(storeid) != content.end()) {
    content.erase(storeid);
  }
}

bool LocalStorage::deleteFile(const Path & filename) {
  Path target = filename;
  if (filename.isRelative()) {
    target = temppath / filename;
  }
  return remove(target.toString().c_str()) == 0;
}

bool LocalStorage::deleteFileAbsolute(const Path & filename) {
  if (filename.isRelative()) {
    return false;
  }
  return remove(filename.toString().c_str()) == 0;
}

bool LocalStorage::deleteRecursive(const Path & path) {
  LocalFile file = getLocalFile(path);
  if (file.isDirectory()) {
    std::shared_ptr<LocalFileList> filelist = getLocalFileList(path);
    std::unordered_map<std::string, LocalFile>::const_iterator it;
    for (it = filelist->begin(); it != filelist->end(); it++) {
      if (!deleteRecursive(path / it->first)) {
        return false;
      }
    }
  }
  return deleteFileAbsolute(path);
}

LocalPathInfo LocalStorage::getPathInfo(const Path & path) {
  int depth = 1;
  return getPathInfo(path, depth);
}

LocalPathInfo LocalStorage::getPathInfo(const Path & path, int currentdepth) {
  LocalFile file = getLocalFile(path);
  if (!file.isDirectory()) {
    return LocalPathInfo(0, 1, 0, file.getSize());
  }
  std::shared_ptr<LocalFileList> filelist = getLocalFileList(path);
  int aggdirs = 1;
  int aggfiles = 0;
  int deepest = currentdepth;
  unsigned long long int aggsize = 0;
  std::unordered_map<std::string, LocalFile>::const_iterator it;
  for (it = filelist->begin(); it != filelist->end(); it++) {
    if (it->second.isDirectory()) {
      LocalPathInfo subpathinfo = getPathInfo(path / it->first, currentdepth + 1);
      aggdirs += subpathinfo.getNumDirs();
      aggfiles += subpathinfo.getNumFiles();
      aggsize += subpathinfo.getSize();
      int depth = subpathinfo.getDepth();
      if (depth > deepest) {
        deepest = depth;
      }
    }
    else {
      ++aggfiles;
      aggsize += it->second.getSize();
    }
  }
  return LocalPathInfo(aggdirs, aggfiles, deepest, aggsize);
}

LocalFile LocalStorage::getLocalFile(const Path & path) {
  std::string name = path.baseName();
  struct stat status;
  lstat((path).toString().c_str(), &status);
  struct passwd * pwd = getpwuid(status.st_uid);
  std::string owner = pwd != NULL ? pwd->pw_name : util::int2Str(status.st_uid);
  struct group * grp = getgrgid(status.st_gid);
  std::string group = grp != NULL ? grp->gr_name : util::int2Str(status.st_gid);
  struct tm tm;
  localtime_r(&status.st_mtime, &tm);
  int year = 1900 + tm.tm_year;
  int month = tm.tm_mon + 1;
  int day = tm.tm_mday;
  int hour = tm.tm_hour;
  int minute = tm.tm_min;
  bool isdir = (status.st_mode & S_IFMT) == S_IFDIR;
  unsigned long long int size = status.st_size;
  return LocalFile(name, size, isdir, owner, group, year, month, day, hour, minute);
}

const Path & LocalStorage::getTempPath() const {
  return temppath;
}

void LocalStorage::setTempPath(const std::string & path) {
  temppath = path;
}

const Path & LocalStorage::getDownloadPath() const {
  return downloadpath;
}

void LocalStorage::setDownloadPath(const Path & path) {
  downloadpath = path;
}

void LocalStorage::storeContent(int storeid, const BinaryData & data) {
  content[storeid] = data;
}

std::shared_ptr<LocalFileList> LocalStorage::getLocalFileList(const Path & path) {
  std::shared_ptr<LocalFileList> filelist;
  DIR * dir = opendir(path.toString().c_str());
  struct dirent * dent;
  while (dir != NULL && (dent = readdir(dir)) != NULL) {
    if (!filelist) {
      filelist = std::make_shared<LocalFileList>(path);
    }
    std::string name = dent->d_name;
    if (name != ".." && name != ".") {
      LocalFile file = getLocalFile(path / name);
      filelist->addFile(file);
    }
  }
  if (dir != NULL) {
    closedir(dir);
  }
  return filelist;
}

unsigned long long int LocalStorage::getFileSize(const Path & file) {
  struct stat status;
  lstat(file.toString().c_str(), &status);
  return status.st_size;
}

bool LocalStorage::getUseActiveModeAddress() const {
  return useactivemodeaddress;
}

const std::string & LocalStorage::getActiveModeAddress() const {
  return activemodeaddress;
}

int LocalStorage::getActivePortFirst() const {
  return activeportfirst;
}

int LocalStorage::getActivePortLast() const {
  return activeportlast;
}

void LocalStorage::setUseActiveModeAddress(bool val) {
  useactivemodeaddress = val;
}

void LocalStorage::setActiveModeAddress(const std::string & addr) {
  activemodeaddress = addr;
}

void LocalStorage::setActivePortFirst(int port) {
  activeportfirst = port;
  currentactiveport = activeportfirst;
}

void LocalStorage::setActivePortLast(int port) {
  activeportlast = port;
  currentactiveport = activeportfirst;
}

int LocalStorage::getNextActivePort() {
  int port = currentactiveport;
  currentactiveport = activeportfirst + ((currentactiveport + 1 - activeportfirst) % (activeportlast + 1 - activeportfirst));
  return port;
}

std::string LocalStorage::getAddress(LocalTransfer * lt) const {
  return getUseActiveModeAddress() ? getActiveModeAddress() : lt->getConn()->getInterfaceAddress();
}
