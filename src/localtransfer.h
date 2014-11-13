#pragma once

#include <fstream>
#include <string>

#include "eventreceiver.h"

#define CHUNK 524288

class TransferMonitor;
class FTPConn;
class LocalStorage;

class LocalTransfer : public EventReceiver {
public:
  LocalTransfer(LocalStorage *);
  void engage(TransferMonitor *, std::string, std::string, std::string, int, bool, FTPConn *);
  void engage(TransferMonitor *, int, std::string, int, bool, FTPConn *);
  bool active() const;
  void FDConnected();
  void FDDisconnected();
  void FDData(char *, unsigned int);
  void FDSSLSuccess();
  void FDSSLFail();
  void FDFail(std::string);
  unsigned long long int size() const;
private:
  void append(char *, unsigned int);
  TransferMonitor * tm;
  bool ssl;
  int sockid;
  bool inuse;
  bool inmemory;
  int storeid;
  FTPConn * ftpconn;
  std::fstream filestream;
  std::string filename;
  unsigned long long int filesize;
  char * buf;
  unsigned int buflen;
  unsigned int bufpos;
  LocalStorage * ls;
};
