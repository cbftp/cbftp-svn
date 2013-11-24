#pragma once

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class SiteLogic;
class UICommunicator;
class RawBuffer;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  bool rawcommandmode;
  bool rawcommandswitch;
  int threads;
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  int connid;
  SiteLogic * sitelogic;
  UICommunicator * uicommunicator;
  RawBuffer * rawbuf;
  CommandHistory history;
};
