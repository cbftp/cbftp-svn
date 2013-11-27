#pragma once

#include <list>
#include <map>

#include "../menuselectoption.h"
#include "../uiwindow.h"

class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;
class UICommunicator;
class SiteManager;
class RemoteCommandHandler;

class GlobalOptionsScreen : public UIWindow {
public:
  GlobalOptionsScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string getSectionButtonText(MenuSelectOptionElement *);
  MenuSelectOption mso;
  RemoteCommandHandler * rch;
  SiteManager * sm;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  UICommunicator * uicommunicator;
  std::map<int, std::string> interfacemap;
  MenuSelectOptionTextArrow * defaultinterface;
};
