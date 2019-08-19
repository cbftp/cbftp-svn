#pragma once

#include "localtransfer.h"

class Path;

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn);
  bool engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, int port, bool ssl, FTPConn* ftpconn);
  void FDConnected(int sockid);
  void FDDisconnected(int sockid);
  void FDData(int sockid, char* data, unsigned int len);
  void FDSSLSuccess(int sockid, const std::string& cipher);
  void FDSSLFail(int sockid);
  void FDSendComplete(int sockid);
  void FDFail(int sockid, const std::string& error);
  unsigned long long int size() const;
private:
  void init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool ssl, int port, bool passivemode);
  void sendChunk();
  unsigned long long int filepos;
};
