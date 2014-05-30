#pragma once

#include <list>
#include <utility>
#include <string>

#include "../uiwindow.h"
#include "../uifilelist.h"

class SiteLogic;
class Site;
class FileList;

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void update();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
  std::list<std::pair<std::string, std::string> > selectionhistory;
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
  bool nuking;
  bool nukesuccess;
  bool nukefailed;
  std::string nuketarget;
  std::string wipetarget;
  std::string wipepath;
  std::string wipefile;
  unsigned int sortmethod;
  Site * site;
  UIFileList list;
  SiteLogic * sitelogic;
  int requestid;
  std::string requestedpath;
  int spinnerpos;
  FileList * filelist;
  void sort();
  void refreshFilelist();
};
