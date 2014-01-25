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
}

void LocalStorage::passiveDownload(TransferMonitor * tm, std::string file, std::string addr, bool ssl, FTPConn * ftpconn) {
  LocalTransfer * lt = NULL;
  for (std::list<LocalTransfer *>::iterator it = localtransfers.begin();
      it != localtransfers.end(); it++) {
    if (!(*it)->active()) {
      lt = *it;
      break;
    }
  }
  if (lt == NULL) {
    lt = new LocalTransfer();
    localtransfers.push_back(lt);
  }
  std::string host = getHostFromPASVString(addr);
  int port = getPortFromPASVString(addr);
  lt->engage(tm, temppath, file, host, port, ssl, ftpconn);
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

int LocalStorage::getFileContent(std::string filename, char * data) {
  std::ifstream filestream;
  filestream.open((temppath + "/" + filename).c_str(), std::ios::binary | std::ios::in);
  filestream.read(data, MAXREAD);
  return filestream.gcount();
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
