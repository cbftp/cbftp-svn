#pragma once

#include <string>
#include <fstream>

#include "core/eventreceiver.h"
#include "path.h"

#define CHUNK 524288

class TransferMonitor;
class FTPConn;

class LocalTransfer : public EventReceiver {
public:
  LocalTransfer();
  bool active() const;
  void FDNew(int);
  void tick(int);
  void openFile(bool);
  int getPort() const;
  virtual unsigned long long int size() const = 0;
  FTPConn * getConn() const;
protected:
  void activate();
  void deactivate();
  bool ssl;
  int sockid;
  bool inmemory;
  bool passivemode;
  int port;
  TransferMonitor * tm;
  bool fileopened;
  FTPConn * ftpconn;
  std::fstream filestream;
  Path path;
  std::string filename;
  char * buf;
  unsigned int buflen;
  unsigned int bufpos;
  bool timeoutticker;
private:
  bool inuse;
};
