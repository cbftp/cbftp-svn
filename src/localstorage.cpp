#include "localstorage.h"

#include "localtransfer.h"
#include "globalcontext.h"
#include "transfermonitor.h"
#include "datafilehandler.h"

#include <stdio.h>
#include <vector>

extern GlobalContext * global;

LocalStorage::LocalStorage() {
  temppath = "/tmp";
  storeidcounter = 0;
}

LocalTransfer * LocalStorage::passiveDownload(TransferMonitor * tm, std::string file, std::string addr, bool ssl, FTPConn * ftpconn) {
  std::string host = getHostFromPASVString(addr);
  int port = getPortFromPASVString(addr);
  LocalTransfer * lt = getAvailableLocalTransfer();
  lt->engage(tm, temppath, file, host, port, ssl, ftpconn);
  return lt;
}

int LocalStorage::passiveDownload(TransferMonitor * tm, std::string addr, bool ssl, FTPConn * ftpconn) {
  int storeid = storeidcounter++;
  std::string host = getHostFromPASVString(addr);
  int port = getPortFromPASVString(addr);
  getAvailableLocalTransfer()->engage(tm, storeid, host, port, ssl, ftpconn);
  return storeid;
}

std::string LocalStorage::getHostFromPASVString(std::string pasv) {
  size_t sep1 = pasv.find(",");
  size_t sep2 = pasv.find(",", sep1 + 1);
  size_t sep3 = pasv.find(",", sep2 + 1);
  size_t sep4 = pasv.find(",", sep3 + 1);
  pasv[sep1] = '.';
  pasv[sep2] = '.';
  pasv[sep3] = '.';
  return pasv.substr(0, sep4);
}

int LocalStorage::getPortFromPASVString(std::string pasv) {
  size_t sep1 = pasv.find(",");
  size_t sep2 = pasv.find(",", sep1 + 1);
  size_t sep3 = pasv.find(",", sep2 + 1);
  size_t sep4 = pasv.find(",", sep3 + 1);
  size_t sep5 = pasv.find(",", sep4 + 1);
  int major = global->str2Int(pasv.substr(sep4 + 1, sep5 - sep4 + 1));
  int minor = global->str2Int(pasv.substr(sep5 + 1));
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

int LocalStorage::getFileContent(std::string filename, char * data) {
  std::ifstream filestream;
  filestream.open((temppath + "/" + filename).c_str(), std::ios::binary | std::ios::in);
  filestream.read(data, MAXREAD);
  return filestream.gcount();
}

int LocalStorage::getStoreContent(int storeid, char ** data) {
  if (content.find(storeid) != content.end()) {
    *data = content[storeid].first;
    return content[storeid].second;
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

std::string LocalStorage::getTempPath() {
  return temppath;
}

void LocalStorage::setTempPath(std::string path) {
  temppath = path;
}

void LocalStorage::storeContent(int storeid, char * buf, int buflen) {
  char * storebuf = (char *) malloc(buflen);
  memcpy(storebuf, buf, buflen);
  content[storeid] = std::pair<char *, int>(storebuf, buflen);
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
  }
}

void LocalStorage::writeState() {
  std::string filetag = "LocalStorage";
  DataFileHandler * filehandler = global->getDataFileHandler();
  filehandler->addOutputLine(filetag, "temppath=" + getTempPath());
}
