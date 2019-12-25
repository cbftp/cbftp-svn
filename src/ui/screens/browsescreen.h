#pragma once

#include <list>
#include <memory>
#include <utility>
#include <string>

#include "../uiwindow.h"
#include "../../path.h"

class BrowseScreenSub;
enum class CompareMode;

enum ViewMode {
  VIEW_NORMAL,
  VIEW_SPLIT,
  VIEW_LOCAL
};

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(Ui *);
  ~BrowseScreen();
  void initialize(unsigned int, unsigned int, ViewMode, const std::string & site = "", const Path path = Path());
  void redraw();
  void update();
  void command(const std::string & command, const std::string & arg);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  bool isInitialized() const;
  void suggestOtherRefresh(BrowseScreenSub* sub);
private:
  void switchSide();
  void closeSide();
  bool keyPressedNoSubAction(unsigned int);
  void toggleCompareListMode(CompareMode mode);
  void clearCompareListMode();
  std::shared_ptr<BrowseScreenSub> left;
  std::shared_ptr<BrowseScreenSub> right;
  std::shared_ptr<BrowseScreenSub> active;
  bool split;
  bool initsplitupdate;
};
