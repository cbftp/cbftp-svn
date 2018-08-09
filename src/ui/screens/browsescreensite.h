#pragma once

#include <list>
#include <utility>
#include <string>

#include "browsescreensub.h"

#include "../uiwindow.h"
#include "../uifilelist.h"
#include "../menuselectoption.h"
#include "../menuselectoptiontextfield.h"
#include "../../path.h"
#include "../../rawbuffer.h"

class SiteLogic;
class Site;
class FileList;
class Ui;
class UIFile;
class BrowseScreenAction;

class BrowseScreenSite : public BrowseScreenSub {
public:
  BrowseScreenSite(Ui *, const std::string &);
  ~BrowseScreenSite();
  BrowseScreenType type() const;
  void redraw(unsigned int, unsigned int, unsigned int);
  void update();
  void command(const std::string & command, const std::string & arg);
  void command(const std::string & command, const std::list<int> & reqids);
  BrowseScreenAction keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  void setFocus(bool);
  void tick(int);
  std::string siteName() const;
  FileList * fileList() const;
  UIFile * selectedFile() const;
  UIFileList * getUIFileList();
  void sort();
  std::list<UIFile *> getSelectedUIFiles() const;
  static void addFileDetails(MenuSelectOption &, unsigned int, unsigned int, const std::string &);
  static void addFileDetails(MenuSelectOption &, unsigned int, unsigned int, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, bool, bool, UIFile * origin);
  static void printFlipped(Ui * ui, const std::shared_ptr<ResizableElement> & re);
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
  bool deleting;
  bool nuking;
  bool mkdiring;
  mutable bool changedsort;
  mutable bool cwdfailed;
  mutable bool wipesuccess;
  mutable bool wipefailed;
  mutable bool deletesuccess;
  mutable bool deletefailed;
  mutable bool nukesuccess;
  mutable bool nukefailed;
  mutable bool mkdirsuccess;
  mutable bool mkdirfailed;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  bool filtermodeinput;
  std::string gotomodestring;
  Path actionpath;
  std::list<std::pair<std::string, bool> > actionfiles;
  unsigned int sortmethod;
  std::shared_ptr<Site> site;
  UIFileList list;
  std::shared_ptr<SiteLogic> sitelogic;
  std::list<int> requestids;
  Path requestedpath;
  mutable int spinnerpos;
  FileList * filelist;
  bool withinraceskiplistreach;
  Path closestracesectionpath;
  std::string separatortext;
  std::list<std::pair<Path, std::string> > selectionhistory;
  bool focus;
  MenuSelectOptionTextField filterfield;
  int temphighlightline;
  RawBuffer cwdrawbuffer;
  bool softselecting;
  void refreshFilelist();
  void disableGotoMode();
  void clearSoftSelects();
  bool handleReadyRequests();
  void printFlipped(const std::shared_ptr<ResizableElement> & re);
};
