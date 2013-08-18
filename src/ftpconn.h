#pragma once

#include <stdlib.h>
#include <list>
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
#include "eventreceiver.h"
#include "sitelogicbase.h"

extern GlobalContext * global;

#define RAWBUFMAXLEN 1024
#define DATABUF 2048

class SiteRace;

class FTPConn : private EventReceiver {
  private:
    IOManager * iom;
    RawBuffer * rawbuf;
    char * databuf;
    unsigned int databuflen;
    unsigned int databufpos;
    int databufcode;
    int id;
    bool processing;
    SiteLogicBase * slb;
    std::string status;
    Site * site;
    int transferstatus;
    int sockfd;
    int state;
    bool aborted;
    FileList * currentfl;
    SiteRace * currentrace;
    std::string currentpath;
    bool protectedmode;
    std::string targetpath;
    bool mkdtarget;
    std::string mkdsect;
    std::string mkdpath;
    std::list<std::string> mkdsubdirs;
    void welcomeReceived();
    void AUTHTLSResponse();
    void USERResponse();
    void PASSResponse();
    void STATResponse();
    void PWDResponse();
    void PROTPResponse();
    void PROTCResponse();
    void RawResponse();
    void CPSVResponse();
    void PASVResponse();
    void PORTResponse();
    void CWDResponse();
    void MKDResponse();
    void PRETRETRResponse();
    void PRETSTORResponse();
    void RETRResponse();
    void RETRComplete();
    void STORResponse();
    void STORComplete();
    void ABORResponse();
    void QUITResponse();
    void sendEcho(std::string);
  public:
    int getId();
    void setId(int);
    std::string getStatus();
    void login();
    void reconnect();
    bool isProcessing();
    FTPConn(SiteLogicBase *, int);
    int updateFileList(FileList *);
    void updateName();
    std::string getCurrentPath();
    void doUSER(bool);
    void doAUTHTLS();
    void doPWD();
    void doPROTP();
    void doPROTC();
    void doRaw(std::string);
    void doSTAT();
    void doSTAT(SiteRace *, FileList *);
    void doCPSV();
    void doPASV();
    void doPORT(std::string);
    void doCWD(std::string);
    void doMKD(std::string);
    void doPRETRETR(std::string);
    void doRETR(std::string);
    void doPRETSTOR(std::string);
    void doSTOR(std::string);
    void abortTransfer();
    void doQUIT();
    void disconnect();
    int getState();
    std::string getConnectedAddress();
    bool getProtectedMode();
    void setMKDCWDTarget(std::string, std::string);
    bool hasMKDCWDTarget();
    std::string getTargetPath();
    std::string getMKDCWDTargetSection();
    std::string getMKDCWDTargetPath();
    void finishMKDCWDTarget();
    std::list<std::string> * getMKDSubdirs();
    RawBuffer * getRawBuffer();
    void FDData(char *, unsigned int);
    void FDConnected();
    void FDDisconnected();
    void FDSSLSuccess();
    void FDSSLFail();
    FileList * currentFileList();
    SiteRace * currentSiteRace();
    void setCurrentSiteRace(SiteRace *);
    void lock();
    void unlock();
};
