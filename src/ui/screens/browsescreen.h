#pragma once

#include <list>
#include <utility>
#include <string>

#include "../uiwindow.h"

#include "../../pointer.h"

class BrowseScreenSub;

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, bool split);
  void redraw();
  void update();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void switchSide();
  void closeSide();
  void keyPressedNoSubAction(unsigned int);
  Pointer<BrowseScreenSub> left;
  Pointer<BrowseScreenSub> right;
  Pointer<BrowseScreenSub> active;
  bool split;
  bool initsplitupdate;
};
