#pragma once

#include <list>
#include <map>

#include "../menuselectoption.h"
#include "../uiwindow.h"

class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;
class SiteManager;
class RemoteCommandHandler;

class GlobalOptionsScreen : public UIWindow {
public:
  GlobalOptionsScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  MenuSelectOption mso;
  RemoteCommandHandler * rch;
  SiteManager * sm;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  std::map<int, std::string> interfacemap;
  MenuSelectOptionTextArrow * defaultinterface;
};
