#pragma once

#include "localtransfer.h"

class LocalStorage;
class Path;

class LocalDownload : public LocalTransfer {
public:
  LocalDownload(LocalStorage* ls);
  void engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn);
  bool engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, int port, bool ssl, FTPConn* ftpconn);
  void engage(TransferMonitor* tm, int storeid, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn);
  bool engage(TransferMonitor* tm, int storeid, bool ipv6, int port, bool ssl, FTPConn* ftpconn);
  unsigned long long int size() const;
  int getStoreId() const;
private:
  void FDConnected(int sockid) override;
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDData(int sockid, char* data, unsigned int len) override;
  void FDSSLSuccess(int sockid, const std::string& cipher) override;
  void FDFail(int sockid, const std::string& error) override;
  void init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool inmemory, int storeid, bool ssl, int port, bool passivemode);
  void append(char* data, unsigned int datalen);
  int storeid;
  unsigned long long int filesize;
  unsigned int bufpos;
  LocalStorage * ls;
};
