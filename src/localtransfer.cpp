#include "localtransfer.h"

#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "globalcontext.h"
#include "iomanager.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"

extern GlobalContext * global;

LocalTransfer::LocalTransfer(LocalStorage * ls) {
  inuse = false;
  buflen = 0;
  this->ls = ls;
}

void LocalTransfer::engage(TransferMonitor * tm, std::string path, std::string filename, std::string addr, int port, bool ssl, FTPConn * ftpconn) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  if (access(path.c_str(), R_OK | W_OK) < 0) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }

  filestream.open((path + "/" + filename).c_str(), std::ios::binary | std::ios::ate | std::ios::out);
  filesize = filestream.tellg();
  bufpos = 0;
  inuse = true;
  inmemory = false;
  this->ssl = ssl;
  global->getIOManager()->registerTCPClientSocket(this, addr, port, &sockid);
}

void LocalTransfer::engage(TransferMonitor * tm, int storeid, std::string addr, int port, bool ssl, FTPConn * ftpconn) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  bufpos = 0;
  filesize = 0;
  inuse = true;
  inmemory = true;
  this->storeid = storeid;
  this->ssl = ssl;
  global->getIOManager()->registerTCPClientSocket(this, addr, port, &sockid);
}

bool LocalTransfer::active() const {
  return inuse;
}

void LocalTransfer::FDConnected() {
  tm->activeReady();
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, (EventReceiver *)ftpconn);
  }
}

void LocalTransfer::FDDisconnected() {
  if (!inmemory) {
    if (bufpos > 0) {
      filestream.write(buf, bufpos);
    }
    filestream.close();
  }
  inuse = false;
  if (inmemory) {
    ls->storeContent(storeid, buf, bufpos);
  }
  tm->targetComplete();
}

void LocalTransfer::FDSSLSuccess() {

}

void LocalTransfer::FDSSLFail() {
  global->getIOManager()->closeSocket(sockid);
  inuse = false;
}

void LocalTransfer::FDData(char * data, unsigned int len) {
  append(data, len);
}

unsigned long long int LocalTransfer::size() const {
  return filesize + bufpos;
}

void LocalTransfer::append(char * data, unsigned int datalen) {
  if (!buflen) {
    buf = (char *) malloc(CHUNK);
    buflen = CHUNK;
  }
  if (bufpos + datalen > buflen) {
    if (inmemory) {
      char * newbuf = (char *) malloc(buflen * 2);
      memcpy(newbuf, buf, bufpos);
      delete buf;
      buf = newbuf;
    }
    else {
      filestream.write(buf, bufpos);
      filesize = filestream.tellg();
      bufpos = 0;
    }
  }
  memcpy(buf + bufpos, data, datalen);
  bufpos += datalen;
}
