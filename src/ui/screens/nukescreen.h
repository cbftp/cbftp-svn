#pragma once

#include <list>

#include "../../core/pointer.h"
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
  void initialize(unsigned int, unsigned int, const std::string &, const std::string &, FileList *);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  int nuke();
  std::string sitestr;
  Pointer<SiteLogic> sitelogic;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string section;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string release;
  Path path;
};
