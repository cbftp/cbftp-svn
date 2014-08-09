#pragma once

#include <list>
#include <string>

#include "eventreceiver.h"

class GlobalContext;
class SiteRace;
class FileList;
class SiteLogic;
class IOManager;
class RawBuffer;
class Site;
class ProxySession;

extern GlobalContext * global;

#define RAWBUFMAXLEN 1024
#define DATABUF 2048

class SiteRace;

class FTPConn : private EventReceiver {
  private:
    IOManager * iom;
    RawBuffer * rawbuf;
    ProxySession * proxysession;
    char * databuf;
    unsigned int databuflen;
    unsigned int databufpos;
    int databufcode;
    int id;
    bool processing;
    SiteLogic * sl;
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
    bool sscnmode;
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
    void PRETLISTResponse();
    void RETRResponse();
    void RETRComplete();
    void STORResponse();
    void STORComplete();
    void ABORResponse();
    void QUITResponse();
    void WIPEResponse();
    void DELEResponse();
    void NUKEResponse();
    void LISTResponse();
    void LISTComplete();
    void SSCNONResponse();
    void SSCNOFFResponse();
    void proxySessionInit(bool);
    void sendEcho(std::string);
  public:
    int getId() const;
    void setId(int);
    std::string getStatus() const;
    void login();
    void reconnect();
    bool isProcessing() const;
    FTPConn(SiteLogic *, int);
    ~FTPConn();
    int updateFileList(FileList *);
    void updateName();
    std::string getCurrentPath() const;
    void doUSER(bool);
    void doAUTHTLS();
    void doPWD();
    void doPROTP();
    void doPROTC();
    void doRaw(std::string);
    void doWipe(std::string, bool);
    void doNuke(std::string, int, std::string);
    void doDELE(std::string);
    void doSTAT();
    void doSTAT(SiteRace *, FileList *);
    void doLIST();
    void doLISTa();
    void prepareLIST();
    void prepareLIST(SiteRace *, FileList *);
    void doSTATla();
    void doSSCN(bool);
    void doCPSV();
    void doPASV();
    void doPORT(std::string);
    void doCWD(std::string);
    void doMKD(std::string);
    void doPRETRETR(std::string);
    void doRETR(std::string);
    void doPRETSTOR(std::string);
    void doSTOR(std::string);
    void doPRETLIST();
    void abortTransfer();
    void doQUIT();
    void doSSLHandshake();
    void disconnect();
    int getState() const;
    std::string getConnectedAddress() const;
    bool getProtectedMode() const;
    bool getSSCNMode() const;
    void setMKDCWDTarget(std::string, std::string);
    bool hasMKDCWDTarget() const;
    std::string getTargetPath() const;
    std::string getMKDCWDTargetSection() const;
    std::string getMKDCWDTargetPath() const;
    void finishMKDCWDTarget();
    std::list<std::string> * getMKDSubdirs();
    RawBuffer * getRawBuffer() const;
    void FDData(char *, unsigned int);
    void FDConnected();
    void FDDisconnected();
    void FDFail(std::string);
    void FDSSLSuccess();
    void FDSSLFail();
    FileList * currentFileList() const;
    SiteRace * currentSiteRace() const;
    void setCurrentSiteRace(SiteRace *);
    void parseFileList(char *, unsigned int);
    bool isConnected() const;
};
