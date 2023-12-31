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
  bool initialize(unsigned int, unsigned int, unsigned int);
  void redraw() override;
  void update() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void deleteFiles(bool);
  std::list<std::string> getIncompleteSites() const;
  std::list<std::string> getDownloadOnlySites() const;
  int getCurrentScope() const;
  std::shared_ptr<Race> race;
  bool smalldirs;
  bool awaitingremovesite;
  bool awaitingremovesitedelownfiles;
  bool awaitingremovesitedelallfiles;
  bool awaitingabort;
  bool awaitingdeleteowninc;
  bool awaitingdeleteownall;
  bool awaitingremovesitefromallspreadjobs;
  unsigned int currnumsubpaths;
  unsigned int currguessedsize;
  unsigned int currincomplete;
  unsigned int longestsubpath;
  std::list<std::string> subpaths;
  MenuSelectOption mso;
  std::map<std::string, int> filetagpos;
  std::map<std::string, std::string> filenametags;
  std::string removesite;
  bool finished;
  int selectsitesmode;
};
