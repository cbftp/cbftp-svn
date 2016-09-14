#pragma once

#include <string>
#include <vector>
#include <utility>

#include "../menuselectoption.h"

#include "browsescreensub.h"

#define BROWSESCREENSELECTOR_HOME "$HOME$"

class Ui;
class BrowseScreenAction;

class BrowseScreenSelector : public BrowseScreenSub {
public:
  BrowseScreenSelector(Ui *);
  ~BrowseScreenSelector();
  BrowseScreenType type() const;
  void redraw(unsigned int, unsigned int, unsigned int);
  void update();
  BrowseScreenAction keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  void setFocus(bool);
private:
  Ui * ui;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  bool focus;
  MenuSelectOption table;
  std::vector<std::pair<std::string, std::string> > entries;
  unsigned int pointer;
  unsigned int currentviewspan;
};
