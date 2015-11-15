#pragma once

#include <list>

#include "../../pointer.h"

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
  void initialize(unsigned int, unsigned int, std::string, std::string, FileList *);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  int nuke();
  std::string sitestr;
  SiteLogic * sitelogic;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string section;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string release;
  std::string path;
};
