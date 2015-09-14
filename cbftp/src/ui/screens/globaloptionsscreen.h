#pragma once

#include <list>
#include <map>

#include "../../pointer.h"

#include "../menuselectoption.h"
#include "../uiwindow.h"

class SiteManager;
class RemoteCommandHandler;
class LocalStorage;
class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;

class GlobalOptionsScreen : public UIWindow {
public:
  GlobalOptionsScreen(Ui *);
  ~GlobalOptionsScreen();
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
  LocalStorage * ls;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  std::map<int, std::string> interfacemap;
  Pointer<MenuSelectOptionTextArrow> defaultinterface;
};
