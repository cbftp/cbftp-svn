#pragma once

#include <string>

class FTPConnectOwner {
public:
  virtual void ftpConnectInfo(int, const std::string &) = 0;
  virtual void ftpConnectSuccess(int) = 0;
  virtual void ftpConnectFail(int) = 0;
  virtual ~FTPConnectOwner() {
  }
};
