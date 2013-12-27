#pragma once

#include "../uiwindow.h"

#include <vector>
#include <string>

class UICommunicator;
class SiteLogic;
class FileList;

#define MAXOPENSIZE 524288

class ViewFileScreen : public UIWindow {
public:
  ViewFileScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  UICommunicator * uicommunicator;
  SiteLogic * sitelogic;
  int requestid;
  FileList * filelist;
  std::string site;
  std::string file;
  bool viewingcontents;
  unsigned int x;
  unsigned int xmax;
  unsigned int y;
  unsigned int ymax;
  std::vector<std::string> contents;
  bool goDown();
  bool goUp();
};
