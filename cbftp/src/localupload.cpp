#include "localupload.h"

#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "globalcontext.h"
#include "iomanager.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "util.h"

extern GlobalContext * global;

LocalUpload::LocalUpload() :
  inuse(false),
  buf((char *) malloc(CHUNK)),
  buflen(CHUNK),
  filepos(0) {

}

void LocalUpload::engage(TransferMonitor * tm, std::string path, std::string filename, std::string addr, int port, bool ssl, FTPConn * ftpconn) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->ssl = ssl;
  filepos = 0;
  inuse = true;
  fileopened = false;
  global->getIOManager()->registerTCPClientSocket(this, addr, port, &sockid);
}

bool LocalUpload::active() const {
  return inuse;
}

void LocalUpload::FDConnected() {
  tm->activeReady();
  openFile();
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, (EventReceiver *)ftpconn);
  }
  else {
    sendChunk();
  }
}

void LocalUpload::FDDisconnected() {
  if (fileopened) {
    filestream.close();
  }
  inuse = false;
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDSSLSuccess() {
  sendChunk();
}

void LocalUpload::sendChunk() {
  util::assert(fileopened);
  filestream.read(buf, buflen);
  int gcount = filestream.gcount();
  if (gcount == 0) {
    filestream.close();
    global->getIOManager()->closeSocket(sockid);
    tm->sourceComplete();
    inuse = false;
  }
  filepos += gcount;
  global->getIOManager()->sendData(sockid, buf, gcount);
}

void LocalUpload::FDSendComplete() {
  sendChunk();
}

void LocalUpload::FDSSLFail() {
  if (fileopened) { // this can theoretically happen mid-transfer
    filestream.close();
  }
  global->getIOManager()->closeSocket(sockid);
  inuse = false;
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDFail(std::string error) {
  inuse = false;
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDData(char * data, unsigned int len) {
  if (fileopened) {
    filestream.close();
  }
  inuse = false;
  global->getIOManager()->closeSocket(sockid);
  tm->sourceError(TM_ERR_OTHER);
}

unsigned long long int LocalUpload::size() const {
  return filepos;
}

void LocalUpload::openFile() {
  if (access(path.c_str(), R_OK) < 0) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }
  filestream.clear();
  filestream.open((path + "/" + filename).c_str(), std::ios::binary | std::ios::in);
  fileopened = true;
}
