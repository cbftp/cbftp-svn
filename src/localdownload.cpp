#include "localdownload.h"

#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "core/iomanager.h"
#include "core/types.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"

extern GlobalContext * global;

LocalDownload::LocalDownload(LocalStorage * ls) :
  inuse(false),
  buflen(0),
  ls(ls) {

}

void LocalDownload::engage(TransferMonitor * tm, std::string path, std::string filename, std::string addr, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl);
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port);
}

void LocalDownload::engage(TransferMonitor * tm, int storeid, std::string addr, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl);
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port);
}

void LocalDownload::init(TransferMonitor * tm, FTPConn * ftpconn, std::string path, std::string filename, bool inmemory, int storeid, bool ssl) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->inmemory = inmemory;
  this->storeid = storeid;
  this->ssl = ssl;
  bufpos = 0;
  filesize = 0;
  inuse = true;
  fileopened = false;
}

bool LocalDownload::active() const {
  return inuse;
}

void LocalDownload::FDConnected(int sockid) {
  tm->activeStarted();
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, (EventReceiver *)ftpconn);
  }
}

void LocalDownload::FDDisconnected(int sockid) {
  if (!inmemory) {
    if (bufpos > 0) {
      if (!fileopened) {
        openFile();
      }
      filestream.write(buf, bufpos);
    }
    if (fileopened) {
      filestream.close();
    }
  }
  inuse = false;
  if (inmemory) {
    BinaryData out(buf, buf + bufpos);
    ls->storeContent(storeid, out);
  }
  tm->targetComplete();
}

void LocalDownload::FDSSLSuccess(int sockid) {
  ftpconn->printCipher(sockid);
}

void LocalDownload::FDSSLFail(int sockid) {
  if (fileopened) { // this can theoretically happen mid-transfer
    filestream.close();
  }
  global->getIOManager()->closeSocket(sockid);
  inuse = false;
  tm->targetError(TM_ERR_OTHER);
}

void LocalDownload::FDFail(int sockid, std::string error) {
  inuse = false;
  tm->targetError(TM_ERR_OTHER);
}

void LocalDownload::FDData(int sockid, char * data, unsigned int len) {
  append(data, len);
}

unsigned long long int LocalDownload::size() const {
  return filesize + bufpos;
}

void LocalDownload::append(char * data, unsigned int datalen) {
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
      if (!fileopened) {
        openFile();
      }
      filestream.write(buf, bufpos);
      filesize = filestream.tellg();
      bufpos = 0;
    }
  }
  memcpy(buf + bufpos, data, datalen);
  bufpos += datalen;
}

int LocalDownload::getStoreId() const {
  return storeid;
}

void LocalDownload::openFile() {
  if (access(path.c_str(), R_OK | W_OK) < 0) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }
  filestream.clear();
  filestream.open((path + "/" + filename).c_str(), std::ios::binary | std::ios::ate | std::ios::out);
  fileopened = true;
}
