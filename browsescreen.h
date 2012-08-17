#pragma once

#include "uiwindow.h"
#include "uicommunicator.h"
#include "termint.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "site.h"
#include "uifilelist.h"

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  Site * site;
  UIFileList list;
  SiteThread * sitethread;
  unsigned int requestid;
  UICommunicator * uicommunicator;
};
