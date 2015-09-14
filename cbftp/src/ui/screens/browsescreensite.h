#pragma once

#include <list>
#include <utility>
#include <string>

#include "browsescreensub.h"

#include "../uiwindow.h"
#include "../uifilelist.h"
#include "../menuselectoption.h"

class SiteLogic;
class Site;
class FileList;
class Ui;
class BrowseScreenAction;

class BrowseScreenSite : public BrowseScreenSub {
public:
  BrowseScreenSite(Ui *, std::string);
  ~BrowseScreenSite();
  BrowseScreenType type() const;
  void redraw(unsigned int, unsigned int, unsigned int);
  void update();
  void command(std::string, std::string);
  BrowseScreenAction keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  void setFocus(bool);
  void tick(int);
  std::string siteName() const;
  FileList * fileList() const;
  UIFile * selectedFile() const;
private:
  Ui * ui;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  MenuSelectOption table;
  unsigned int currentviewspan;
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
  bool withinraceskiplistreach;
  std::string closestracesectionpath;
  std::string separatortext;
  std::list<std::pair<std::string, std::string> > selectionhistory;
  bool focus;
  void sort();
  void refreshFilelist();
  size_t countDirLevels(std::string);
  void addFileDetails(unsigned int, std::string);
  void addFileDetails(unsigned int, std::string, std::string, std::string, std::string, std::string, bool, bool);
  void disableGotoMode();
};
