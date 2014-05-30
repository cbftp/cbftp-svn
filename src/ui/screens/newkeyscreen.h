#pragma once

#define SHORTESTKEY 4

#include "../uiwindow.h"
#include "../menuselectoption.h"

class NewKeyScreen : public UIWindow {
public:
  NewKeyScreen(Ui *);
  void initialize(unsigned int, unsigned int);
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
  bool tooshort;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
};
