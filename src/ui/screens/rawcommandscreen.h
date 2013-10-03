#pragma once

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"

class UICommunicator;
class RawBuffer;
class SiteLogic;

class RawCommandScreen : public UIWindow {
public:
  RawCommandScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  SiteLogic * sitelogic;
  UICommunicator * uicommunicator;
  RawBuffer * rawbuf;
};
