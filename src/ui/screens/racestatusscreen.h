#pragma once

#include <list>
#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class UICommunicator;
class Race;

class RaceStatusScreen : public UIWindow {
public:
  RaceStatusScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  char getFileChar(bool, bool, bool);
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
  UICommunicator * uicommunicator;
  MenuSelectOption mso;
  std::map<std::string, int> filetagpos;
  std::map<std::string, std::string> filenametags;
  std::string removesite;
};
