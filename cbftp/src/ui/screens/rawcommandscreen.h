#pragma once

#include "../../core/pointer.h"

#include "../../path.h"

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class RawBuffer;
class SiteLogic;

class RawCommandScreen : public UIWindow {
public:
  RawCommandScreen(Ui * ui);
  void initialize(unsigned int, unsigned int, const std::string &, const Path &, const std::string &);
  void initialize(unsigned int, unsigned int, const std::string &);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  std::string selection;
  Path path;
  bool hasselection;
  Pointer<SiteLogic> sitelogic;
  RawBuffer * rawbuf;
  CommandHistory history;
};
