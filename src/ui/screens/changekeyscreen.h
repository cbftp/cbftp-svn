#pragma once

#define SHORTESTKEY 4

#include "../uiwindow.h"
#include "../menuselectoption.h"

class UICommunicator;
class MenuSelectOptionElement;

class ChangeKeyScreen : public UIWindow {
public:
  ChangeKeyScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool mismatch;
  bool oldmismatch;
  bool tooshort;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  std::string operation;
  UICommunicator * uicommunicator;
};
