#pragma once

#include <memory>
#include <string>
#include <list>
#include <utility>

#include "browsescreensub.h"

#include "../menuselectoption.h"
#include "../uifilelist.h"
#include "../menuselectoptiontextfield.h"

#include "../../localstorage.h"
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
  std::shared_ptr<LocalFileList> fileList() const;
  UIFile * selectedFile() const;
  UIFileList * getUIFileList();
private:
  void refreshFilelist();
  bool handleReadyRequests();
  void disableGotoMode();
  void gotoPath(int requestid, const Path & path);
  void clearSoftSelects();
  void viewCursored();
  bool keyDown();
  Ui * ui;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  unsigned int currentviewspan;
  bool focus;
  MenuSelectOption table;
  UIFileList list;
  std::list<BrowseScreenRequest> requests;
  mutable int spinnerpos;
  mutable int tickcount;
  bool resort;
  UIFileList::SortMethod sortmethod;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  bool filtermodeinput;
  bool filtermodeinputregex;
  std::string gotomodestring;
  std::list<std::pair<Path, std::string> > selectionhistory;
  std::shared_ptr<LocalFileList> filelist;
  MenuSelectOptionTextField filterfield;
  int temphighlightline;
  bool softselecting;
  LastInfo lastinfo;
  std::string lastinfotarget;
  bool refreshfilelistafter;
  unsigned long long int freespace;
};
