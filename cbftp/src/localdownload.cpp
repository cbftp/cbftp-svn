#include "localdownload.h"

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "core/iomanager.h"
#include "core/types.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "path.h"

LocalDownload::LocalDownload(LocalStorage* ls) :
  ls(ls)
{
}

void LocalDownload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, port, true);
  bool resolving;
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port, resolving, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4);
}

bool LocalDownload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, port, false);
  sockid = global->getIOManager()->registerTCPServerSocket(this, port, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4);
  return sockid != -1;
}

void LocalDownload::engage(TransferMonitor* tm, int storeid, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, port, true);
  bool resolving;
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port, resolving, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4);
}

bool LocalDownload::engage(TransferMonitor* tm, int storeid, bool ipv6, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, port, false);
  sockid = global->getIOManager()->registerTCPServerSocket(this, port, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4);
  return sockid != -1;
}

void LocalDownload::init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool inmemory, int storeid, bool ssl, int port, bool passivemode) {
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
  fileopened = false;
  activate();
}

void LocalDownload::FDConnected(int sockid) {
  if (sockid != this->sockid) {
    return;
  }
  if (passivemode) {
    tm->activeStarted();
  }
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, ftpconn->getSockId());
  }
}

void LocalDownload::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (sockid != this->sockid) {
    return;
  }
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
  deactivate();
  if (inmemory) {
    Core::BinaryData out(buf, buf + bufpos);
    ls->storeContent(storeid, out);
  }
  if (reason != Core::DisconnectType::ERROR) {
    tm->targetComplete();
  }
  else {
    tm->targetError(TM_ERR_RETRSTOR_COMPLETE);
  }
  this->sockid = -1;
}

void LocalDownload::FDSSLSuccess(int sockid, const std::string& cipher) {
  if (sockid != this->sockid) {
    return;
  }
  ftpconn->printCipher(cipher);
  bool sessionreused = global->getIOManager()->getSSLSessionReused(sockid);
  tm->sslDetails(cipher, sessionreused);
}

void LocalDownload::FDData(int sockid, char* data, unsigned int len) {
  if (sockid != this->sockid) {
    return;
  }
  append(data, len);
}

void LocalDownload::FDFail(int sockid, const std::string& error) {
  if (sockid != this->sockid) {
    return;
  }
  deactivate();
  tm->targetError(TM_ERR_OTHER);
  this->sockid = -1;
}

unsigned long long int LocalDownload::size() const {
  return filesize + bufpos;
}

void LocalDownload::append(char* data, unsigned int datalen) {
  if (!buflen) {
    buf = (char*) malloc(CHUNK);
    buflen = CHUNK;
  }
  if (bufpos + datalen > buflen) {
    if (inmemory) {
      char * newbuf = (char*) malloc(buflen * 2);
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
