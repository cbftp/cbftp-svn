#include "ftpconn.h"

#include <stdlib.h>

#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "filelist.h"
#include "site.h"
#include "siterace.h"
#include "globalcontext.h"
#include "rawbuffer.h"
#include "iomanager.h"
#include "sitelogicbase.h"
#include "eventlog.h"
#include "proxymanager.h"
#include "proxy.h"
#include "proxysession.h"

FTPConn::FTPConn(SiteLogicBase * slb, int id) {
  this->slb = slb;
  this->id = id;
  this->site = slb->getSite();
  this->status = "disconnected";
  processing = false;
  rawbuf = new RawBuffer(RAWBUFMAXLEN, site->getName(), global->int2Str(id));
  proxysession = new ProxySession();
  iom = global->getIOManager();
  databuflen = DATABUF;
  databuf = (char *) malloc(databuflen);
  databufpos = 0;
  protectedmode = false;
  mkdtarget = false;
  currentpath = "";
  state = 0;
}

FTPConn::~FTPConn() {
  delete rawbuf;
  delete databuf;
  delete proxysession;
}

int FTPConn::getId() {
  return id;
}

void FTPConn::setId(int id) {
  this->id = id;
  rawbuf->setId(id);
}

std::string FTPConn::getStatus() {
  return status;
}

void FTPConn::login() {

  if (state != 0) {
    return;
  }
  protectedmode = false;
  mkdtarget = false;
  databufpos = 0;
  processing = true;
  currentpath = "";
  Proxy * proxy = NULL;
  int proxytype = site->getProxyType();
  if (proxytype == SITE_PROXY_USE) {
    proxy = global->getProxyManager()->getProxy(site->getProxy());
  }
  else if (proxytype == SITE_PROXY_GLOBAL) {
    proxy = global->getProxyManager()->getDefaultProxy();
  }
  if (proxy == NULL) {
    rawbuf->writeLine("[Connecting to " + site->getAddress() + ":" + site->getPort() + "]");
    state = 1;
    sockfd = iom->registerTCPClientSocket(this, site->getAddress(), global->str2Int(site->getPort()));
  }
  else {
    rawbuf->writeLine("[Connecting to proxy " + proxy->getAddr() + ":" + proxy->getPort() + "]");
    state = 100;
    proxysession->prepare(proxy, site->getAddress(), site->getPort());
    sockfd = iom->registerTCPClientSocket(this, proxy->getAddr(), global->str2Int(proxy->getPort()));
  }
  if (sockfd < 0) {
    state = 0;
  }
}

void FTPConn::proxySessionInit(bool connect) {
  if (!connect) {
    proxysession->received(databuf, databufpos);
  }
  switch (proxysession->instruction()) {
    case PROXYSESSION_SEND_CONNECT:
      rawbuf->writeLine("[Connecting to " + site->getAddress() + ":" + site->getPort() + " through proxy]");
      iom->sendData(sockfd, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND:
      iom->sendData(sockfd, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      rawbuf->writeLine("[Connection established]");
      state = 2;
      break;
    case PROXYSESSION_ERROR:
      rawbuf->writeLine("[Proxy error: " + proxysession->getErrorMessage() + "]");
      state = 0;
      iom->closeSocket(sockfd);
      rawbuf->writeLine("[Disconnected]");
      this->status = "disconnected";
      slb->unexpectedResponse(id);
      break;
  }
}

void FTPConn::FDConnected() {
  rawbuf->writeLine("[Connection established]");
  if (state == 100) {
    proxySessionInit(true);
  }
  else {
    state = 2;
  }
}

void FTPConn::FDDisconnected() {
  if (state != 0) {
    rawbuf->writeLine("[Disconnected]");
    this->status = "disconnected";
    state = 0;
    slb->disconnected(id);
  }
}

void FTPConn::FDFail(std::string error) {
  rawbuf->writeLine("[" + error + "]");
  slb->connectFailed(1);
}

void FTPConn::FDSSLSuccess() {
  rawbuf->writeLine("[Cipher: " + iom->getCipher(sockfd) + "]");
  doUSER(false);
}

void FTPConn::FDSSLFail() {

}

void FTPConn::FDData(char * data, unsigned int datalen) {
  if (state != 6 && state != 100) {
    rawbuf->write(std::string(data, datalen));
  }
  if (databufpos + datalen > databuflen) {
    databuflen = databuflen * 2;
    char * newdatabuf = (char *) malloc(databuflen);
    memcpy(newdatabuf, databuf, databufpos);
    delete databuf;
    databuf = newdatabuf;
  }
  memcpy(databuf + databufpos, data, datalen);
  databufpos += datalen;
  bool messagecomplete = false;
  char * loc;
  if (state == 100) {
    messagecomplete = true;
    databufcode = 1337;
  }
  else {
    if(databuf[databufpos - 1] == '\n') {
      loc = databuf + databufpos - 5;
      while (loc >= databuf) {
        if (*loc >= 48 && *loc <= 57 && *(loc+1) >= 48 && *(loc+1) <= 57 && *(loc+2) >= 48 && *(loc+2) <= 57) {
          if ((*(loc+3) == ' ' || *(loc+3) == '\n') && (loc == databuf || *(loc-1) == '\n')) {
            messagecomplete = true;
            databufcode = atoi(std::string(loc, 3).data());
            break;
          }
        }
        --loc;
      }
    }
  }
  if (messagecomplete) {
    if (databufcode == 550) {
      // workaround for a glftpd bug causing an extra row '550 Unable to load your own user file!.' on retr/stor
      if (*(loc+4) == 'U' && *(loc+5) == 'n' && *(loc+28) == 'u' && *(loc+33) == 'f') {
        databufpos = 0;
        return;
      }
    }
    switch(state) {
      case 2: // awaiting welcome on connect
        welcomeReceived();
        break;
      case 3: // awaiting AUTH TLS response
        AUTHTLSResponse();
        break;
      case 4: // awaiting USER response
        USERResponse();
        break;
      case 5: // awaiting PASS response
        PASSResponse();
        break;
      case 6: // awaiting STAT response
        STATResponse();
        break;
      case 7: // awaiting PWD response
        PWDResponse();
        break;
      case 8: // awaiting PROT P response
        PROTPResponse();
        break;
      case 9:  // awaiting PROT C response
        PROTCResponse();
        break;
      case 10: // awaiting raw response
        RawResponse();
        break;
      case 11: // awaiting CPSV response
        CPSVResponse();
        break;
      case 12: // awaiting PASV response
        PASVResponse();
        break;
      case 13: // awaiting PORT response
        PORTResponse();
        break;
      case 14: // awaiting CWD response
        CWDResponse();
        break;
      case 15: // awaiting MKD response
        MKDResponse();
        break;
      case 16: // awaiting PRET RETR response
        PRETRETRResponse();
        break;
      case 17: // awaiting PRET STOR response
        PRETSTORResponse();
        break;
      case 18: // awaiting RETR response
        RETRResponse();
        break;
      case 19: // awaiting RETR complete
        RETRComplete();
        break;
      case 20: // awaiting STOR response
        STORResponse();
        break;
      case 21: // awaiting STOR complete
        STORComplete();
        break;
      case 22: // awaiting ABOR response
        ABORResponse();
        break;
      case 23: // awaiting QUIT response
        QUITResponse();
        break;
      case 24: // awaiting loginkilling USER response
        USERResponse();
        break;
      case 25: // awaiting loginkilling PASS response
        PASSResponse();
        break;
      case 28: // awaiting WIPE response
        WIPEResponse();
        break;
      case 29: // awaiting DELE response
        DELEResponse();
        break;
      case 100: // negotiating proxy session
        proxySessionInit(false);
        break;
    }
    databufpos = 0;
  }
}

void FTPConn::sendEcho(std::string data) {
  rawbuf->writeLine(data);
  processing = true;
  status = data;
  iom->sendData(sockfd, data + "\n");
}

void FTPConn::welcomeReceived() {
  if (databufcode == 220) {
    if (site->SSL()) {
      state = 3;
      sendEcho("AUTH TLS");
    }
    else {
      doUSER(false);
    }
  }
  else {
    rawbuf->writeLine("[Unknown response]");
    state = 0;
    processing = false;
    iom->closeSocket(sockfd);
    rawbuf->writeLine("[Disconnected]");
    this->status = "disconnected";
    slb->unexpectedResponse(id);
  }
}

void FTPConn::AUTHTLSResponse() {
  if (databufcode == 234) {
    iom->negotiateSSL(sockfd);
  }
  else {
    rawbuf->writeLine("[Unknown response]");
    state = 0;
    processing = false;
    iom->closeSocket(sockfd);
    slb->TLSFailed(id);
  }
}

void FTPConn::doUSER(bool killer) {
  if (!killer) {
    state = 4;
  }
  else {
    state = 24;
  }
  sendEcho((std::string("USER ") + (killer ? "!" : "") + site->getUser()).data());
}

void FTPConn::USERResponse() {
  if (databufcode == 331) {
    std::string pass = site->getPass();
    std::string passc = "";
    for (unsigned int i = 0; i < pass.length(); i++) passc.append("*");
    std::string output = "PASS " + std::string(passc);
    rawbuf->writeLine(output);
    status = output;
    if (state == 4) {
      state = 5;
    }
    else {
      state = 25;
    }
    iom->sendData(sockfd, std::string("PASS ") + site->getPass()  + "\n");
  }
  else {
    processing = false;
    bool sitefull = false;
    bool simultaneous = false;
    std::string reply = std::string(databuf, databufpos);
    if (databufcode == 530 || databufcode == 550) {
      if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
        sitefull = true;
      }
      else if (reply.find("simultaneous") != std::string::npos) {
        simultaneous = true;
      }
    }
    if (sitefull) {
      slb->userDeniedSiteFull(id);
    }
    else if (state == 4) {
      if (simultaneous) {
        slb->userDeniedSimultaneousLogins(id);
      }
      else {
        slb->userDenied(id);
      }
    }
    else {
      slb->loginKillFailed(id);
    }
  }
}

void FTPConn::PASSResponse() {
  processing = false;
  if (databufcode == 230) {
    this->status = "connected";
    state = 5;
    slb->commandSuccess(id);
  }
  else {
    bool sitefull = false;
    bool simultaneous = false;
    std::string reply = std::string(databuf, databufpos);
    if (databufcode == 530 || databufcode == 550) {
      if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
        sitefull = true;
      }
      else if (reply.find("simultaneous") != std::string::npos) {
        simultaneous = true;
      }
    }
    if (sitefull) {
      slb->userDeniedSiteFull(id);
    }
    else if (state == 5) {
      if (simultaneous) {
        slb->userDeniedSimultaneousLogins(id);
      }
      else {
        slb->passDenied(id);
      }
    }
    else {
      slb->loginKillFailed(id);
    }
  }
}

void FTPConn::reconnect() {
  if (state != 0) {
    iom->closeSocket(sockfd);
    rawbuf->writeLine("[Disconnected]");
    this->status = "disconnected";
    state = 0;
  }
  login();
}

void FTPConn::doSTAT() {
  doSTAT(NULL, new FileList(site->getUser(), currentpath));
}

void FTPConn::doSTAT(SiteRace * race, FileList * filelist) {
  state = 6;
  currentrace = race;
  currentfl = filelist;
  sendEcho("STAT -l");
}

void FTPConn::STATResponse() {
  processing = false;
  if (databufcode == 211 || databufcode == 213) {
    char * loc = databuf, * start;
    unsigned int files = 0;
    int touch = rand();
    while (loc + 4 < databuf + databufpos && !(*(loc + 1) == '2' && *(loc + 2) == '1' && *(loc + 4) == ' ')) {
      if (*(loc + 1) == '2' && *(loc + 2) == '1' && *(loc + 4) == '-') loc += 4;
      start = loc + 1;
      while (loc < databuf + databufpos && loc - start < 50) {
        start = loc + 1;
        while(loc < databuf + databufpos && *++loc != '\n');
      }
      if (loc - start >= 50) {
        if (currentfl->updateFile(std::string(start, loc - start), touch)) {
          files++;
        }
      }
    }
    if (currentfl->getSize() > files) {
      currentfl->cleanSweep(touch);
    }
    std::string output = "[File list retrieved]";
    rawbuf->writeLine(output);
    if (!currentfl->isFilled()) currentfl->setFilled();
    slb->listRefreshed(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::updateName() {
  rawbuf->rename(site->getName());
}

std::string FTPConn::getCurrentPath() {
  return currentpath;
}
void FTPConn::doPWD() {
  state = 7;
  sendEcho("PWD");
}

void FTPConn::PWDResponse() {
  processing = false;
  if (databufcode == 257) {
    std::string line(databuf, databufpos);
    int loc = 0;
    while(line[++loc] != '"');
    int start = loc + 1;
    while(line[++loc] != '"');
    slb->gotPath(id, line.substr(start, loc - start));
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doPROTP() {
  state = 8;
  sendEcho("PROT P");
}

void FTPConn::PROTPResponse() {
  processing = false;
  if (databufcode == 200) {
    protectedmode = true;
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doPROTC() {
  state = 9;
  sendEcho("PROT C");
}

void FTPConn::PROTCResponse() {
  processing = false;
  if (databufcode == 200) {
    protectedmode = false;
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doRaw(std::string command) {
  state = 10;
  sendEcho(command.c_str());
}

void FTPConn::doWipe(std::string path, bool recursive) {
  state = 28;
  sendEcho(std::string("SITE WIPE ") + (recursive ? "-r " : "") + path);
}

void FTPConn::doDELE(std::string path) {
  state = 29;
  sendEcho("DELE " + path);
}

void FTPConn::RawResponse() {
  processing = false;
  std::string ret = std::string(databuf, databufpos);
  slb->rawCommandResultRetrieved(id, ret);
}

void FTPConn::WIPEResponse() {
  processing = false;
  if (databufcode == 200) {
    std::string data = std::string(databuf, databufpos);
    if (data.find("successfully") != std::string::npos) {
      slb->commandSuccess(id);
      return;
    }
  }
  slb->commandFail(id);
}

void FTPConn::DELEResponse() {
  processing = false;
  if (databufcode == 250) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doCPSV() {
  state = 11;
  sendEcho("CPSV");
}

void FTPConn::CPSVResponse() {
  processing = false;
  if (databufcode == 227) {
    std::string data = std::string(databuf, databufpos);
    size_t start = data.find('(') + 1;
    size_t end = data.find(')');
    slb->gotPassiveAddress(id, data.substr(start, end-start));
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doPASV() {
  state = 12;
  sendEcho("PASV");
}

void FTPConn::PASVResponse() {
  processing = false;
  if (databufcode == 227) {
    std::string data = std::string(databuf, databufpos);
    size_t start = data.find('(') + 1;
    size_t end = data.find(')');
    slb->gotPassiveAddress(id, data.substr(start, end-start));
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doPORT(std::string addr) {
  state = 13;
  sendEcho(("PORT " + addr).c_str());
}

void FTPConn::PORTResponse() {
  processing = false;
  if (databufcode == 200) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doCWD(std::string path) {
  if (path == currentpath) {
    global->getEventLog()->log("FTPConn " + site->getName() + global->int2Str(id),
        "WARNING: Noop CWD requested: " + path);
    return;
  }
  targetpath = path;
  state = 14;
  sendEcho(("CWD " + path).c_str());
}

void FTPConn::CWDResponse() {
  processing = false;
  if (databufcode == 250) {
    currentpath = targetpath;
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doMKD(std::string dir) {
  targetpath = dir;
  state = 15;
  sendEcho(("MKD " + dir).c_str());
}

void FTPConn::MKDResponse() {
  processing = false;
  if (databufcode == 257) {
    slb->commandSuccess(id);
  }
  else {
    if (databufcode == 550 &&
        std::string(databuf, databufpos).find("File exist") !=
            std::string::npos) {
      slb->commandSuccess(id);
    }
    else {
      slb->commandFail(id);
    }
  }
}

void FTPConn::doPRETRETR(std::string file) {
  state = 16;
  sendEcho(("PRET RETR " + file).c_str());
}

void FTPConn::PRETRETRResponse() {
  processing = false;
  if (databufcode == 200) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doPRETSTOR(std::string file) {
  state = 17;
  sendEcho(("PRET STOR " + file).c_str());
}

void FTPConn::PRETSTORResponse() {
  processing = false;
  if (databufcode == 200) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doRETR(std::string file) {
  state = 18;
  sendEcho(("RETR " + file).c_str());
}

void FTPConn::RETRResponse() {
  if (databufcode == 150) {
    slb->commandSuccess(id);
    state = 19;
  }
  else {
    processing = false;
    slb->commandFail(id);
  }
}

void FTPConn::RETRComplete() {
  processing = false;
  if (databufcode == 226) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::doSTOR(std::string file) {
  state = 20;
  sendEcho(("STOR " + file).c_str());
}

void FTPConn::STORResponse() {
  if (databufcode == 150) {
    slb->commandSuccess(id);
    state = 21;
  }
  else {
    processing = false;
    slb->commandFail(id);
  }
}

void FTPConn::STORComplete() {
  processing = false;
  if (databufcode == 226) {
    slb->commandSuccess(id);
  }
  else {
    slb->commandFail(id);
  }
}

void FTPConn::abortTransfer() {
  state = 22;
  sendEcho("ABOR");
}

void FTPConn::ABORResponse() {
  processing = false;
  slb->commandSuccess(id);
}

void FTPConn::doQUIT() {
  state = 23;
  sendEcho("QUIT");
}

void FTPConn::QUITResponse() {
  processing = false;
  disconnect();
}

void FTPConn::disconnect() {
  if (state != 0) {
    state = 0;
    iom->closeSocket(sockfd);
    this->status = "disconnected";
    rawbuf->writeLine("[Disconnected]");
  }
}

RawBuffer * FTPConn::getRawBuffer() {
  return rawbuf;
}

int FTPConn::getState() {
  return state;
}

std::string FTPConn::getConnectedAddress() {
  return iom->getSocketAddress(sockfd);
}

bool FTPConn::getProtectedMode() {
  return protectedmode;
}

void FTPConn::setMKDCWDTarget(std::string section, std::string subpath) {
  mkdtarget = true;
  mkdsect = section;
  mkdpath = subpath;
  size_t lastpos = 0;
  mkdsubdirs.clear();
  while (true) {
    size_t splitpos = mkdpath.find("/", lastpos);
    if (splitpos != std::string::npos) {
      mkdsubdirs.push_back(mkdpath.substr(lastpos, splitpos - lastpos));
    }
    else {
      mkdsubdirs.push_back(mkdpath.substr(lastpos));
      break;
    }
    lastpos = splitpos + 1;
  }
}

void FTPConn::finishMKDCWDTarget() {
  mkdtarget = false;
}

bool FTPConn::hasMKDCWDTarget() {
  return mkdtarget;
}

std::string FTPConn::getTargetPath() {
  return targetpath;
}

std::string FTPConn::getMKDCWDTargetSection() {
  return mkdsect;
}
std::string FTPConn::getMKDCWDTargetPath() {
  return mkdpath;
}

std::list<std::string> * FTPConn::getMKDSubdirs() {
  return &mkdsubdirs;
}

FileList * FTPConn::currentFileList() {
  return currentfl;
}

SiteRace * FTPConn::currentSiteRace() {
  return currentrace;
}

void FTPConn::setCurrentSiteRace(SiteRace * race) {
  currentrace = race;
}

bool FTPConn::isProcessing() {
  return processing;
}

void FTPConn::lock() {
  slb->lock();
}

void FTPConn::unlock() {
  slb->unlock();
}
