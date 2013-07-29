#pragma once

#include <vector>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../ftpconn.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../termint.h"

extern GlobalContext * global;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string sitename;
  Site * site;
  UICommunicator * uicommunicator;
};
