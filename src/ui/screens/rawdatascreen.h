#pragma once

#include "../../core/pointer.h"

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
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  static void printRawBufferLines(Ui *, RawBuffer *, unsigned int, unsigned int, unsigned int);
  static void printRawBufferLines(Ui *, RawBuffer *, unsigned int, unsigned int, unsigned int, bool, unsigned int, unsigned int);
private:
  static bool skipCodePrint(const std::string &);
  bool rawcommandmode;
  bool rawcommandswitch;
  int threads;
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  int connid;
  Pointer<SiteLogic> sitelogic;
  RawBuffer * rawbuf;
  CommandHistory history;
};
