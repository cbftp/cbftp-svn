#include "localstorage.h"

#include "localtransfer.h"
#include "globalcontext.h"
#include "transfermonitor.h"
#include "datafilehandler.h"
#include "localfilelist.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

extern GlobalContext * global;

LocalStorage::LocalStorage() {
  temppath = "/tmp";
  downloadpath = std::string(getenv("HOME")) + "/Downloads";
  storeidcounter = 0;
}

LocalTransfer * LocalStorage::passiveDownload(TransferMonitor * tm, std::string file, std::string addr, bool ssl, FTPConn * ftpconn) {
  return passiveDownload(tm, temppath, file, addr, ssl, ftpconn);
}

LocalTransfer * LocalStorage::passiveDownload(TransferMonitor * tm, std::string path, std::string file, std::string addr, bool ssl, FTPConn * ftpconn) {
  std::string host = getHostFromPASVString(addr);
  int port = getPortFromPASVString(addr);
  LocalTransfer * lt = getAvailableLocalTransfer();
  lt->engage(tm, path, file, host, port, ssl, ftpconn);
  return lt;
}

LocalTransfer * LocalStorage::passiveDownload(TransferMonitor * tm, std::string addr, bool ssl, FTPConn * ftpconn) {
  int storeid = storeidcounter++;
  std::string host = getHostFromPASVString(addr);
  int port = getPortFromPASVString(addr);
  LocalTransfer * lt = getAvailableLocalTransfer();
  lt->engage(tm, storeid, host, port, ssl, ftpconn);
  return lt;
}

std::string LocalStorage::getHostFromPASVString(std::string pasv) const {
  size_t sep1 = pasv.find(",");
  size_t sep2 = pasv.find(",", sep1 + 1);
  size_t sep3 = pasv.find(",", sep2 + 1);
  size_t sep4 = pasv.find(",", sep3 + 1);
  pasv[sep1] = '.';
  pasv[sep2] = '.';
  pasv[sep3] = '.';
  return pasv.substr(0, sep4);
}

int LocalStorage::getPortFromPASVString(std::string pasv) const {
  size_t sep1 = pasv.find(",");
  size_t sep2 = pasv.find(",", sep1 + 1);
  size_t sep3 = pasv.find(",", sep2 + 1);
  size_t sep4 = pasv.find(",", sep3 + 1);
  size_t sep5 = pasv.find(",", sep4 + 1);
  int major = util::str2Int(pasv.substr(sep4 + 1, sep5 - sep4 + 1));
  int minor = util::str2Int(pasv.substr(sep5 + 1));
  return major * 256 + minor;
}

LocalTransfer * LocalStorage::getAvailableLocalTransfer() {
  LocalTransfer * lt = NULL;
  for (std::list<LocalTransfer *>::iterator it = localtransfers.begin(); it != localtransfers.end(); it++) {
    if (!(*it)->active()) {
      lt = *it;
      break;
    }
  }
  if (lt == NULL) {
    lt = new LocalTransfer(this);
    localtransfers.push_back(lt);
  }
  return lt;
}

int LocalStorage::getFileContent(std::string filename, char * data) const {
  std::ifstream filestream;
  filestream.open((temppath + "/" + filename).c_str(), std::ios::binary | std::ios::in);
  filestream.read(data, MAXREAD);
  return filestream.gcount();
}

int LocalStorage::getStoreContent(int storeid, char ** data) const {
  std::map<int, std::pair<char *, int> >::const_iterator it = content.find(storeid);
  if (it != content.end()) {
    *data = it->second.first;
    return it->second.second;
  }
  return 0;
}

void LocalStorage::purgeStoreContent(int storeid) {
  if (content.find(storeid) != content.end()) {
    delete content[storeid].first;
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

void LocalStorage::setTempPath(std::string path) {
  temppath = path;
}

std::string LocalStorage::getDownloadPath() const {
  return downloadpath;
}

void LocalStorage::setDownloadPath(std::string path) {
  downloadpath = path;
}

void LocalStorage::storeContent(int storeid, char * buf, int buflen) {
  char * storebuf = (char *) malloc(buflen);
  memcpy(storebuf, buf, buflen);
  content[storeid] = std::pair<char *, int>(storebuf, buflen);
}

Pointer<LocalFileList> LocalStorage::getLocalFileList(std::string path) {
  Pointer<LocalFileList> filelist(new LocalFileList(path));
  DIR * dir = opendir(path.c_str());
  struct dirent * dent;
  while ((dent = readdir(dir)) != NULL) {
    struct stat status;
    stat(dent->d_name, &status);
    std::string name = dent->d_name;
    if (name != ".." && name != ".") {
      filelist->addFile(name, status.st_size, status.st_mode & S_IFDIR);
    }
  }
  closedir(dir);
  return filelist;
}


bool LocalStorage::directoryExistsWritable(std::string path) {
  return directoryExistsAccessible(path, true);
}

bool LocalStorage::directoryExistsReadable(std::string path) {
  return directoryExistsAccessible(path, false);
}

bool LocalStorage::directoryExistsAccessible(std::string path, bool write) {
  int how = write ? R_OK | W_OK : R_OK;
  if (access(path.c_str(), how) == 0) {
    struct stat status;
    stat(path.c_str(), &status);
    if (status.st_mode & S_IFDIR) {
      return true;
    }
  }
  return false;
}

bool LocalStorage::createDirectory(std::string path) {
  return createDirectory(path, false);
}

bool LocalStorage::createDirectory(std::string path, bool privatedir) {
  mode_t mode = privatedir ? 0700 : 0755;
  return mkdir(path.c_str(), mode) >= 0;
}

bool LocalStorage::createDirectoryRecursive(std::string path) {
  std::list<std::string> pathdirs;
  if (path.length() == 0) {
    return false;
  }
  if (path[0] == '/') {
    path = path.substr(1);
  }
  size_t slashpos;
  while ((slashpos = path.find("/")) != std::string::npos) {
    pathdirs.push_back(path.substr(0, slashpos));
    path = path.substr(slashpos + 1);
  }
  if (path.length()) {
    pathdirs.push_back(path);
  }
  std::string partialpath;
  while (pathdirs.size() > 0) {
    partialpath += "/" + pathdirs.front();
    pathdirs.pop_front();
    if (!directoryExistsReadable(partialpath)) {
      if (!createDirectory(partialpath)) {
        return false;
      }
    }
  }
  return true;
}

void LocalStorage::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("LocalStorage", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("temppath")) {
      setTempPath(value);
    }
    if (!setting.compare("downloadpath")) {
      setDownloadPath(value);
    }
  }
}

void LocalStorage::writeState() {
  std::string filetag = "LocalStorage";
  DataFileHandler * filehandler = global->getDataFileHandler();
  filehandler->addOutputLine(filetag, "temppath=" + getTempPath());
  filehandler->addOutputLine(filetag, "downloadpath=" + getDownloadPath());
}
