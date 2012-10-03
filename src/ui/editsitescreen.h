#pragma once

#include "../sitemanager.h"
#include "../site.h"

#include "uiwindow.h"
#include "uicommunicator.h"
#include "menuselectoption.h"
#include "menuselectoptionelement.h"
#include "termint.h"

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  Site * site;
  Site modsite;
  std::string operation;
  UICommunicator * uicommunicator;
};
