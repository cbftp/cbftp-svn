#pragma once

#include "localtransfer.h"

class Path;

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn);
  bool engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, int port, bool ssl, FTPConn* ftpconn);
  unsigned long long int size() const;
private:
  void FDConnected(int sockid) override;
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDData(int sockid, char* data, unsigned int len) override;
  void FDSSLSuccess(int sockid, const std::string& cipher) override;
  void FDSendComplete(int sockid) override;
  void FDFail(int sockid, const std::string& error) override;
  void init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool ssl, int port, bool passivemode);
  void sendChunk();
  unsigned long long int filepos;
};
