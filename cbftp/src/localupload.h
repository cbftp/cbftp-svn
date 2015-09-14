#pragma once

#include <fstream>
#include <string>

#include "localtransfer.h"

class TransferMonitor;
class FTPConn;
class LocalStorage;

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor *, std::string, std::string, std::string, int, bool, FTPConn *);
  bool active() const;
  void FDConnected();
  void FDDisconnected();
  void FDData(char *, unsigned int);
  void FDSSLSuccess();
  void FDSSLFail();
  void FDFail(std::string);
  void FDSendComplete();
  unsigned long long int size() const;
private:
  void sendChunk();
  void openFile();
  TransferMonitor * tm;
  bool ssl;
  int sockid;
  bool inuse;
  bool fileopened;
  FTPConn * ftpconn;
  std::fstream filestream;
  std::string path;
  std::string filename;
  char * buf;
  unsigned int buflen;
  unsigned long long int filepos;
};
