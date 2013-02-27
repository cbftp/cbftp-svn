#pragma once

#include "../../ftpthread.h"
#include "../../rawbuffer.h"
#include "../../sitethread.h"
#include "../../sitethreadmanager.h"
#include "../../globalcontext.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptiontextfield.h"

extern GlobalContext * global;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  bool rawcommandmode;
  bool rawcommandswitch;
  int threads;
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  int threadid;
  SiteThread * sitethread;
  UICommunicator * uicommunicator;
  RawBuffer * rawbuf;
};
