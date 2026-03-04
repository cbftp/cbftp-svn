#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Path;

class MoveScreen : public UIWindow {
public:
  MoveScreen(Ui *);
  ~MoveScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & site, const std::string& items, const Path& srcpath, const std::string& dstpath, const std::string& firstitem);
  void redraw() override;
  std::string getInfoLabel() const override;
  std::string getLegendText() const override;
private:
  bool keyPressed(unsigned int ch) override;
  void onKeyPressedActive(unsigned int ch) override;
  std::string site;
  MenuSelectOption mso;
  std::string srcpath;
  std::string items;
  std::string firstitem;
};
