#pragma once

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class RawBuffer;
class SiteLogic;

class RawCommandScreen : public UIWindow {
public:
  RawCommandScreen(Ui * ui);
  void initialize(unsigned int, unsigned int, std::string, std::string);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  std::string selection;
  bool hasselection;
  SiteLogic * sitelogic;
  RawBuffer * rawbuf;
  CommandHistory history;
};
