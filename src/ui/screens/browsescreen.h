#pragma once

#include <list>

#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../site.h"
#include "../../globalcontext.h"
#include "../../skiplist.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../termint.h"
#include "../uifilelist.h"
#include "../stringpair.h"

extern GlobalContext * global;

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
  std::list<StringPair> selectionhistory;
private:
  unsigned int currentviewspan;
  unsigned int sliderstart;
  unsigned int slidersize;
  bool virgin;
  bool resort;
  int tickcount;
  bool changedsort;
  unsigned int sortmethod;
  Site * site;
  UIFileList list;
  SiteLogic * sitelogic;
  int requestid;
  UICommunicator * uicommunicator;
  std::string requestedpath;
  int spinnerpos;
  void sort();
};
