#pragma once

#include <string>
#include <list>
#include <utility>

#include "browsescreensub.h"

#include "../menuselectoption.h"
#include "../uifilelist.h"
#include "../menuselectoptiontextfield.h"

#include "../../core/pointer.h"
#include "../../path.h"

class Ui;
class LocalFileList;

class BrowseScreenLocal : public BrowseScreenSub {
public:
  BrowseScreenLocal(Ui *);
  ~BrowseScreenLocal();
  BrowseScreenType type() const;
  void redraw(unsigned int, unsigned int, unsigned int);
  void update();
  void command(const std::string &, const std::string &);
  BrowseScreenAction keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  void setFocus(bool);
  void tick(int);
  Pointer<LocalFileList> fileList() const;
  UIFile * selectedFile() const;
  UIFileList * getUIFileList();
  void sort();
private:
  void disableGotoMode();
  void gotoPath(const Path &);
  void clearSoftSelects();
  Ui * ui;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  unsigned int currentviewspan;
  bool focus;
  MenuSelectOption table;
  UIFileList list;
  mutable bool changedsort;
  mutable bool cwdfailed;
  mutable bool deletesuccess;
  mutable bool deletefailed;
  mutable int tickcount;
  bool resort;
  int sortmethod;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  bool filtermodeinput;
  std::string gotomodestring;
  std::list<std::pair<Path, std::string> > selectionhistory;
  Pointer<LocalFileList> filelist;
  Path targetpath;
  MenuSelectOptionTextField filterfield;
  int temphighlightline;
  bool deleting;
  Path actionpath;
  std::list<std::pair<std::string, bool> > actionfiles;
  bool softselecting;
};
