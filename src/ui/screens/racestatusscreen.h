#pragma once

#include <list>
#include <map>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Race;

class RaceStatusScreen : public UIWindow {
public:
  RaceStatusScreen(Ui *);
  ~RaceStatusScreen();
  void initialize(unsigned int, unsigned int, unsigned int);
  void redraw();
  void update();
  void command(const std::string &, const std::string &);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  char getFileChar(bool, bool, bool, bool) const;
  void deleteFiles(bool);
  std::shared_ptr<Race> race;
  bool smalldirs;
  bool awaitingremovesite;
  bool awaitingremovesitedelownfiles;
  bool awaitingremovesitedelallfiles;
  bool awaitingabort;
  unsigned int currnumsubpaths;
  unsigned int currguessedsize;
  unsigned int longestsubpath;
  std::list<std::string> subpaths;
  MenuSelectOption mso;
  std::map<std::string, int> filetagpos;
  std::map<std::string, std::string> filenametags;
  std::string removesite;
  std::string defaultlegendtext;
  std::string finishedlegendtext;
  bool finished;
  int selectsitesmode;
};
