#pragma once

#include "uiwindow.h"
#include "uiwindowcommand.h"
#include "sitemanager.h"
#include "site.h"
#include "menuselectoption.h"
#include "menuselectoptionelement.h"
#include "termint.h"

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(WINDOW *, UIWindowCommand *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
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
  UIWindowCommand * windowcommand;
};
