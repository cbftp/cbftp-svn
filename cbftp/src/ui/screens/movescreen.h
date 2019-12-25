#pragma once

#include <list>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Path;

class MoveScreen : public UIWindow {
public:
  MoveScreen(Ui *);
  ~MoveScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & site, const std::string& items, const Path& srcpath, const std::string& dstpath);
  void update();
  void redraw();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string site;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string srcpath;
  std::string items;
};
