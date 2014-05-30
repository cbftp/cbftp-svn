#pragma once

#include <list>
#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Race;

class RaceStatusScreen : public UIWindow {
public:
  RaceStatusScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void update();
  void command(std::string);
  void keyPressed(unsigned int);
  char getFileChar(bool, bool, bool, bool);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  Race * race;
  bool smalldirs;
  bool awaitingremovesite;
  bool awaitingabort;
  unsigned int currnumsubpaths;
  unsigned int currguessedsize;
  unsigned int longestsubpath;
  std::string release;
  std::string sitestr;
  std::list<std::string> subpaths;
  MenuSelectOption mso;
  std::map<std::string, int> filetagpos;
  std::map<std::string, std::string> filenametags;
  std::string removesite;
};
