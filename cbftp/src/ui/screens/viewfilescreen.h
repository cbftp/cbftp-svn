#pragma once

#include "../uiwindow.h"

#include "../../pointer.h"
#include "../../encoding.h"

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
  ~ViewFileScreen();
  void initialize(unsigned int, unsigned int, std::string, std::string, FileList *);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
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
  bool downloadattempted;
  std::vector<std::string > rawcontents;
  std::vector<std::basic_string<unsigned int> > translatedcontents;
  std::string path;
  int pid;
  encoding::Encoding encoding;
  bool goDown();
  bool goUp();
  void printTransferInfo();
  void translate();
};
