#pragma once

#include "localtransfer.h"

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor *, const std::string &, const std::string &, const std::string &, int, bool, FTPConn *);
  bool engage(TransferMonitor *, const std::string &, const std::string &, int, bool, FTPConn *);
  bool active() const;
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSSLSuccess(int);
  void FDSSLFail(int);
  void FDSendComplete(int);
  void FDFail(int, std::string);
  unsigned long long int size() const;
private:
  void init(TransferMonitor *, FTPConn *, const std::string &, const std::string &, bool, int, bool);
  void sendChunk();
  unsigned long long int filepos;
};
