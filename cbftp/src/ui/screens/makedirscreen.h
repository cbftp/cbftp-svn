#pragma once

#include <list>

#include "../../core/pointer.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../uifilelist.h"

class MenuSelectOptionElement;
class UIFileList;

class MakeDirScreen : public UIWindow {
public:
  MakeDirScreen(Ui *);
  ~MakeDirScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & site,  UIFileList & filelist);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void tryMakeDir();
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string site;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string release;
  UIFileList filelist;
  bool alreadyexists;
};
