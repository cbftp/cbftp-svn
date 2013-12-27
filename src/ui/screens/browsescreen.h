#pragma once

#include <list>

#include "../uiwindow.h"
#include "../uifilelist.h"

class UICommunicator;
class StringPair;
class SiteLogic;
class Site;
class FileList;

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
  std::list<StringPair> selectionhistory;
private:
  unsigned int currentviewspan;
  unsigned int sliderstart;
  unsigned int slidersize;
  bool virgin;
  bool resort;
  int tickcount;
  bool changedsort;
  bool cwdfailed;
  bool wipe;
  bool wiperecursive;
  bool wipesuccess;
  bool wipefailed;
  bool deleting;
  bool deletingrecursive;
  bool deletesuccess;
  bool deletefailed;
  std::string wipetarget;
  std::string wipepath;
  std::string wipefile;
  unsigned int sortmethod;
  Site * site;
  UIFileList list;
  SiteLogic * sitelogic;
  int requestid;
  UICommunicator * uicommunicator;
  std::string requestedpath;
  int spinnerpos;
  FileList * filelist;
  void sort();
};
