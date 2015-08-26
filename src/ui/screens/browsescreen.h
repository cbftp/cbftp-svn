#pragma once

#include <list>
#include <utility>
#include <string>

#include "../uiwindow.h"

#include "../../pointer.h"

class BrowseScreenSub;

enum ViewMode {
  VIEW_NORMAL,
  VIEW_SPLIT,
  VIEW_LOCAL
};

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(Ui *);
  ~BrowseScreen();
  void initialize(unsigned int, unsigned int, ViewMode, std::string);
  void redraw();
  void update();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  bool isInitialized() const;
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
