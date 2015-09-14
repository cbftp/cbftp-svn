#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class ProxyManager;
class MenuSelectOptionTextArrow;

class ProxyOptionsScreen : public UIWindow {
public:
  ProxyOptionsScreen(Ui *);
  ~ProxyOptionsScreen();
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void command(std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool defaultset;
  std::string deleteproxy;
  std::string editproxy;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  MenuSelectOption msop;
  ProxyManager * pm;
  Pointer<MenuSelectOptionTextArrow> useproxy;
};
