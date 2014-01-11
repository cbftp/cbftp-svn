#include "localtransfer.h"

#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "globalcontext.h"
#include "iomanager.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"

extern GlobalContext * global;

LocalTransfer::LocalTransfer() {
  inuse = false;
  buflen = 0;
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
  this->ssl = ssl;
  global->getIOManager()->registerTCPClientSocket(this, addr, port, &sockfd);
}

bool LocalTransfer::active() {
  return inuse;
}

void LocalTransfer::FDConnected() {
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockfd, (EventReceiver *)ftpconn);
  }
}

void LocalTransfer::FDDisconnected() {
  global->getIOManager()->closeSocket(sockfd);
  if (bufpos > 0) {
    filestream.write(buf, bufpos);
  }
  filestream.close();
  inuse = false;
  tm->targetComplete();
}

void LocalTransfer::FDSSLSuccess() {

}

void LocalTransfer::FDSSLFail() {
  global->getIOManager()->closeSocket(sockfd);
  inuse = false;
}

void LocalTransfer::FDData(char * data, unsigned int len) {
  append(data, len);
}

unsigned long long int LocalTransfer::size() {
  return filesize + bufpos;
}

void LocalTransfer::append(char * data, unsigned int datalen) {
  if (!buflen) {
    buf = (char *) malloc(CHUNK);
    buflen = CHUNK;
  }
  if (bufpos + datalen > buflen) {
    filestream.write(buf, bufpos);
    filesize = filestream.tellg();
    bufpos = 0;
  }
  memcpy(buf + bufpos, data, datalen);
  bufpos += datalen;
}
