#pragma once

#include <list>
#include <utility>
#include <string>

#include "../uiwindow.h"
#include "../uifilelist.h"

#include "../../eventreceiver.h"

class SiteLogic;
class Site;
class FileList;

class BrowseScreen : public UIWindow, public EventReceiver {
public:
  BrowseScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void update();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  std::list<std::pair<std::string, std::string> > selectionhistory;
  void tick(int);
private:
  unsigned int currentviewspan;
  unsigned int sliderstart;
  unsigned int slidersize;
  bool virgin;
  bool resort;
  mutable int tickcount;
  bool wipe;
  bool wiperecursive;
  bool deleting;
  bool deletingrecursive;
  bool nuking;
  mutable bool changedsort;
  mutable bool cwdfailed;
  mutable bool wipesuccess;
  mutable bool wipefailed;
  mutable bool deletesuccess;
  mutable bool deletefailed;
  mutable bool nukesuccess;
  mutable bool nukefailed;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  std::string gotomodestring;
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
  mutable int spinnerpos;
  FileList * filelist;
  void sort();
  void refreshFilelist();
};
