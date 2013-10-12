#pragma once

#include "eventreceiver.h"

class Site;

class SiteLogicBase : public EventReceiver {
private:
public:
  virtual void tick(int) = 0;
  virtual void connectFailed(int) = 0;
  virtual void userDenied(int) = 0;
  virtual void userDeniedSiteFull(int) = 0;
  virtual void userDeniedSimultaneousLogins(int) = 0;
  virtual void loginKillFailed(int) = 0;
  virtual void passDenied(int) = 0;
  virtual void TLSFailed(int) = 0;
  virtual void listRefreshed(int) = 0;
  virtual void unexpectedResponse(int) = 0;
  virtual Site * getSite() = 0;
  virtual void commandSuccess(int) = 0;
  virtual void commandFail(int) = 0;
  virtual void gotPath(int, std::string) = 0;
  virtual void rawCommandResultRetrieved(int, std::string) = 0;
  virtual void gotPassiveAddress(int, std::string) = 0;
  virtual void timedout(int) = 0;
  virtual void disconnected(int) = 0;
};
