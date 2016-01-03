#pragma once

#include <list>
#include <string>

#include "eventreceiver.h"

#define STATE_DISCONNECTED 0
#define STATE_CONNECTING 1
#define STATE_WELCOME 2
#define STATE_AUTH_TLS 3
#define STATE_USER 4
#define STATE_PASS 5
#define STATE_STAT 6
#define STATE_PWD 7
#define STATE_PROT_P 8
#define STATE_PROT_C 9
#define STATE_RAW 10
#define STATE_CPSV 11
#define STATE_PASV 12
#define STATE_PORT 13
#define STATE_CWD 14
#define STATE_MKD 15
#define STATE_PRET_RETR 16
#define STATE_PRET_STOR 17
#define STATE_RETR 18
#define STATE_RETR_COMPLETE 19
#define STATE_STOR 20
#define STATE_STOR_COMPLETE 21
#define STATE_ABOR 22
#define STATE_QUIT 23
#define STATE_USER_LOGINKILL 24
#define STATE_PASS_LOGINKILL 25
#define STATE_WIPE 28
#define STATE_DELE 29
#define STATE_NUKE 30
#define STATE_LIST 31
#define STATE_PRET_LIST 32
#define STATE_LIST_COMPLETE 33
#define STATE_SSCN_ON 34
#define STATE_SSCN_OFF 35
#define STATE_SSL_HANDSHAKE 36
#define STATE_PASV_ABORT 37
#define STATE_PBSZ 38
#define STATE_TYPEI 39
#define STATE_PROXY 100

#define PROT_UNSET 5483
#define PROT_C 5484
#define PROT_P 5485

class SiteRace;
class FileList;
class SiteLogic;
class IOManager;
class RawBuffer;
class Site;
class ProxySession;
class CommandOwner;

#define RAWBUFMAXLEN 1024
#define DATABUF 2048

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
    int sockid;
    int state;
    bool aborted;
    FileList * currentfl;
    CommandOwner * currentco;
    std::string currentpath;
    int protectedmode;
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
    void PASVAbortResponse();
    void PBSZ0Response();
    void TYPEIResponse();
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
    void doSTAT(CommandOwner *, FileList *);
    void doLIST();
    void doLISTa();
    void prepareLIST();
    void prepareLIST(CommandOwner *, FileList *);
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
    void abortTransferPASV();
    void doPBSZ0();
    void doTYPEI();
    void doQUIT();
    void doSSLHandshake();
    void disconnect();
    int getState() const;
    std::string getConnectedAddress() const;
    int getProtectedMode() const;
    bool getSSCNMode() const;
    void setMKDCWDTarget(std::string, std::string);
    bool hasMKDCWDTarget() const;
    std::string getTargetPath() const;
    std::string getMKDCWDTargetSection() const;
    std::string getMKDCWDTargetPath() const;
    void finishMKDCWDTarget();
    std::list<std::string> * getMKDSubdirs();
    RawBuffer * getRawBuffer() const;
    void FDData(int, char *, unsigned int);
    void FDConnected(int);
    void FDDisconnected(int);
    void FDFail(int, std::string);
    void FDSSLSuccess(int);
    void FDSSLFail(int);
    void printCipher(int);
    FileList * currentFileList() const;
    CommandOwner * currentCommandOwner() const;
    void setCurrentCommandOwner(CommandOwner *);
    void parseFileList(char *, unsigned int);
    bool isConnected() const;
};
