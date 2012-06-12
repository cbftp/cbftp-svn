#pragma once
#include "uiwindow.h"
#include "uiwindowcommand.h"
#include "globalcontext.h"
#include "site.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "ftpthread.h"
#include <vector>
#include <iostream>

extern GlobalContext * global;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(WINDOW *, UIWindowCommand *, int, int);
  void redraw();
  void update();
  void keyPressed(int);
private:
  std::string sitename;
  Site * site;
  UIWindowCommand * windowcommand;
};
