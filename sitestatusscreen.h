#pragma once

#include <vector>

#include "uiwindow.h"
#include "uicommunicator.h"
#include "globalcontext.h"
#include "site.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "ftpthread.h"
#include "termint.h"

extern GlobalContext * global;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  std::string sitename;
  Site * site;
  UICommunicator * uicommunicator;
};
