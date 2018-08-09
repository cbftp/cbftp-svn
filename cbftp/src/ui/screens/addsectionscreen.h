#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Site;
class MenuSelectOptionElement;
class Path;

class AddSectionScreen : public UIWindow {
public:
  AddSectionScreen(Ui *);
  ~AddSectionScreen();
  void initialize(unsigned int, unsigned int, std::string, const Path &);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::shared_ptr<Site> modsite;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
