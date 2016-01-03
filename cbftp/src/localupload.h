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
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSSLSuccess(int);
  void FDSSLFail(int);
  void FDFail(int, std::string);
  void FDSendComplete(int);
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
