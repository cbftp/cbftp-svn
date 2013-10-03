#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class UICommunicator;
class Site;

class AddSectionScreen : public UIWindow {
public:
  AddSectionScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  Site * modsite;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
};
