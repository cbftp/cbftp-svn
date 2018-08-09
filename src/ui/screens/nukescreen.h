#pragma once

#include <list>
#include <memory>

#include "../../path.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Site;
class SiteLogic;
class FileList;

class NukeScreen : public UIWindow {
public:
  NukeScreen(Ui *);
  ~NukeScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & sitestr, const std::list<std::pair<std::string, bool> > & items, const Path & path);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void nuke();
  void nuke(int multiplier, const std::string & reason);
  std::string sitestr;
  std::shared_ptr<SiteLogic> sitelogic;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string section;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::list<std::pair<std::string, bool> > items;
  Path path;
};
