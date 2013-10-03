#pragma once

#include "../../site.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../menusection.h"

class UICommunicator;
class FocusableArea;

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  MenuSection ms;
  Site * site;
  Site modsite;
  std::string operation;
  UICommunicator * uicommunicator;
};
