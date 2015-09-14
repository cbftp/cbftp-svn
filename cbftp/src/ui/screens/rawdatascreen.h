#pragma once

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class SiteLogic;
class RawBuffer;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
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
  RawBuffer * rawbuf;
  CommandHistory history;
};
