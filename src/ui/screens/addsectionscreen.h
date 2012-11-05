#pragma once

#include "../../site.h"
#include "../../sitemanager.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../menuselectoption.h"
#include "../menuselectoptionelement.h"
#include "../termint.h"

class AddSectionScreen : public UIWindow {
public:
  AddSectionScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  Site * modsite;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
};
