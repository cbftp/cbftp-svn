#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class ProxyManager;
class MenuSelectOptionTextArrow;

class ProxyOptionsScreen : public UIWindow {
public:
  ProxyOptionsScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void command(std::string);
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
private:
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool defaultset;
  std::string deleteproxy;
  std::string editproxy;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  MenuSelectOption msop;
  ProxyManager * pm;
  MenuSelectOptionTextArrow * useproxy;
};
