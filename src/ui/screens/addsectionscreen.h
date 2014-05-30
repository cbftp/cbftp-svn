#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Site;

class AddSectionScreen : public UIWindow {
public:
  AddSectionScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, std::string);
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
};
