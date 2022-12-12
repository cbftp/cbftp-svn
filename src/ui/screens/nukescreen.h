#pragma once

#include <list>
#include <memory>

#include "../../path.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class NukeScreen : public UIWindow {
public:
  NukeScreen(Ui *);
  ~NukeScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & sitestr, const std::string & items, const Path & path);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void nuke();
  void nuke(int multiplier, const std::string & reason);
  std::string sitestr;
  bool active;
  std::string section;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string items;
  Path path;
};
