#pragma once

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptiontextfield.h"

extern GlobalContext * global;

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
