#pragma once

#include "../uiwindow.h"
#include "../../pointer.h"

#include <vector>
#include <string>

class TransferStatus;
class SiteLogic;
class FileList;
class Ui;

#define MAXOPENSIZE 524288

class ViewFileScreen : public UIWindow {
public:
  ViewFileScreen(Ui * ui);
  void initialize(unsigned int, unsigned int, std::string, std::string, FileList *);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  SiteLogic * sitelogic;
  Pointer<TransferStatus> ts;
  FileList * filelist;
  std::string site;
  std::string file;
  bool viewingcontents;
  unsigned int x;
  unsigned int xmax;
  unsigned int y;
  unsigned int ymax;
  unsigned long long int size;
  bool hasnodisplay;
  bool externallyviewable;
  bool download;
  bool legendupdated;
  std::vector<std::string> contents;
  std::string path;
  int pid;
  bool goDown();
  bool goUp();
};
