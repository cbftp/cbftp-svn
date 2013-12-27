#pragma once

#include <fstream>
#include <string>

#include "eventreceiver.h"

#define CHUNK 524288

class TransferMonitor;
class FTPConn;

class LocalTransfer : public EventReceiver {
public:
  LocalTransfer();
  void engage(TransferMonitor *, std::string, std::string, std::string, int, bool, FTPConn *);
  bool active();
  void FDConnected();
  void FDDisconnected();
  void FDData(char *, unsigned int);
  void FDSSLSuccess();
  void FDSSLFail();
  unsigned long long int size();
private:
  void append(char *, unsigned int);
  TransferMonitor * tm;
  bool ssl;
  int sockfd;
  bool inuse;
  FTPConn * ftpconn;
  std::fstream filestream;
  std::string filename;
  unsigned long long int filesize;
  char * buf;
  unsigned int buflen;
  unsigned int bufpos;
};
