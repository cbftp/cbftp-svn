#pragma once

#include <list>
#include <map>
#include <memory>

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
  void update() override;
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
private:
  MenuSelectOption mso;
  RemoteCommandHandler * rch;
  SiteManager * sm;
  LocalStorage * ls;
  std::map<int, std::string> interfacemap;
  std::shared_ptr<MenuSelectOptionTextArrow> bindinterface;
};
