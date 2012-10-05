#pragma once

#include <list>

#include "../sitethread.h"
#include "../sitethreadmanager.h"
#include "../site.h"

#include "uiwindow.h"
#include "uicommunicator.h"
#include "termint.h"
#include "uifilelist.h"
#include "selectionpair.h"

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::list<SelectionPair> selectionhistory;
private:
  unsigned int currentviewspan;
  bool virgin;
  bool resort;
  unsigned int sortmethod;
  Site * site;
  UIFileList list;
  SiteThread * sitethread;
  unsigned int requestid;
  UICommunicator * uicommunicator;
  void sort();
};
