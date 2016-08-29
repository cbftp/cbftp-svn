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

LocalTransfer * LocalStorage::passiveModeDownload(TransferMonitor * tm, const std::string & path, const std::string & file, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
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

LocalTransfer * LocalStorage::passiveModeUpload(TransferMonitor * tm, const std::string & path, const std::string & file, const std::string & host, int port, bool ssl, FTPConn * ftpconn) {
  LocalUpload * lu = getAvailableLocalUpload();
  lu->engage(tm, path, file, host, port, ssl, ftpconn);
  return lu;
}

LocalTransfer * LocalStorage::activeModeDownload(TransferMonitor * tm, const std::string & path, const std::string & file, bool ssl, FTPConn * ftpconn) {
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

LocalTransfer * LocalStorage::activeModeUpload(TransferMonitor * tm, const std::string & path, const std::string & file, bool ssl, FTPConn * ftpconn) {
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
  return getFileContent(temppath + "/" + filename);
}

BinaryData LocalStorage::getFileContent(const std::string & filename) const {
  std::ifstream filestream;
  filestream.open(filename.c_str(), std::ios::binary | std::ios::in);
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
  throw;
}

void LocalStorage::purgeStoreContent(int storeid) {
  if (content.find(storeid) != content.end()) {
    content.erase(storeid);
  }
}

void LocalStorage::deleteFile(std::string filename) {
  if (filename.length() > 0 && filename[0] != '/') {
    filename = temppath + "/" + filename;
  }
  remove(filename.c_str());
}

std::string LocalStorage::getTempPath() const {
  return temppath;
}

void LocalStorage::setTempPath(const std::string & path) {
  temppath = path;
}

std::string LocalStorage::getDownloadPath() const {
  return downloadpath;
}

void LocalStorage::setDownloadPath(const std::string & path) {
  downloadpath = path;
}

void LocalStorage::storeContent(int storeid, const BinaryData & data) {
  content[storeid] = data;
}

Pointer<LocalFileList> LocalStorage::getLocalFileList(const std::string & path) {
  Pointer<LocalFileList> filelist;
  DIR * dir = opendir(path.c_str());
  struct dirent * dent;
  while (dir != NULL && (dent = readdir(dir)) != NULL) {
    if (!filelist) {
      filelist = makePointer<LocalFileList>(path);
    }
    std::string name = dent->d_name;
    struct stat status;
    lstat(std::string(path + "/" + name).c_str(), &status);
    if (name != ".." && name != ".") {
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
      filelist->addFile(name, status.st_size, (status.st_mode & S_IFMT) ==
                        S_IFDIR, owner, group, year, month, day, hour, minute);
    }
  }
  if (dir != NULL) {
    closedir(dir);
  }
  return filelist;
}

unsigned long long int LocalStorage::getFileSize(const std::string & file) {
  struct stat status;
  lstat(file.c_str(), &status);
  return status.st_size;
}

bool LocalStorage::getUseActiveModeAddress() const {
  return useactivemodeaddress;
}

std::string LocalStorage::getActiveModeAddress() const {
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
