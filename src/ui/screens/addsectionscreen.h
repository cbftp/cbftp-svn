#pragma once

#include "../../pointer.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../menuselectoptionelement.h"

class Site;

class AddSectionScreen : public UIWindow {
public:
  AddSectionScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, std::string);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  Site * modsite;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
