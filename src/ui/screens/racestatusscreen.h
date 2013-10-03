#pragma once

#include <list>
#include <map>

#include "../uiwindow.h"

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
  bool spaceous;
  bool smalldirs;
  unsigned int currnumsubpaths;
  unsigned int currguessedsize;
  unsigned int longestsubpath;
  std::string release;
  std::string sitestr;
  std::list<std::string> subpaths;
  UICommunicator * uicommunicator;
  std::map<std::string, int> filetagpos;
};
