#pragma once

#include "../../proxy.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class UICommunicator;
class MenuSelectOptionElement;

class EditProxyScreen : public UIWindow {
public:
  EditProxyScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  Proxy * proxy;
  Proxy modproxy;
  MenuSelectOptionTextArrow * authmethod;
  int latestauthmethod;
  std::string operation;
  UICommunicator * uicommunicator;
};
