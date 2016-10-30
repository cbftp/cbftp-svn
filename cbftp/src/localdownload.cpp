#include "localdownload.h"

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "core/iomanager.h"
#include "core/types.h"
#include "core/tickpoke.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "path.h"

LocalDownload::LocalDownload(LocalStorage * ls) :
  ls(ls)
{
}

void LocalDownload::engage(TransferMonitor * tm, const Path & path, const std::string & filename, const std::string & addr, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, port, true);
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port);
}

bool LocalDownload::engage(TransferMonitor * tm, const Path & path, const std::string & filename, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, port, false);
  sockid = global->getIOManager()->registerTCPServerSocket(this, port);
  return sockid != -1;
}

void LocalDownload::engage(TransferMonitor * tm, int storeid, const std::string & addr, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, port, true);
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port);
}

bool LocalDownload::engage(TransferMonitor * tm, int storeid, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, port, false);
  sockid = global->getIOManager()->registerTCPServerSocket(this, port);
  return sockid != -1;
}

void LocalDownload::init(TransferMonitor * tm, FTPConn * ftpconn, const Path & path, const std::string & filename, bool inmemory, int storeid, bool ssl, int port, bool passivemode) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->inmemory = inmemory;
  this->storeid = storeid;
  this->ssl = ssl;
  this->port = port;
  this->passivemode = passivemode;
  bufpos = 0;
  filesize = 0;
  inuse = true;
  fileopened = false;
  if (!passivemode) {
    global->getTickPoke()->startPoke(this, "LocalDownload", 5000, 0);
  }
}

void LocalDownload::FDConnected(int sockid) {
  if (passivemode) {
    tm->activeStarted();
  }
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, (EventReceiver *)ftpconn);
  }
}

void LocalDownload::FDDisconnected(int sockid) {
  if (!inmemory) {
    if (bufpos > 0) {
      if (!fileopened) {
        openFile(false);
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
  tm->cipher(global->getIOManager()->getCipher(sockid));
}

void LocalDownload::FDSSLFail(int sockid) {
  if (fileopened) { // this can theoretically happen mid-transfer
    filestream.close();
  }
  global->getIOManager()->closeSocket(sockid);
  inuse = false;
  tm->targetError(TM_ERR_OTHER);
}

void LocalDownload::FDData(int sockid, char * data, unsigned int len) {
  append(data, len);
}

void LocalDownload::FDFail(int sockid, std::string error) {
  inuse = false;
  if (sockid != -1) {
    tm->targetError(TM_ERR_OTHER);
  }
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
        openFile(false);
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
