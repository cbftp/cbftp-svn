#pragma once

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../menuselectoption.h"

class FocusableArea;
class Ui;
class PreparedRace;

class MainScreen : public UIWindow {
public:
  MainScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void command(std::string);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void addPreparedRaceTableRow(unsigned int, MenuSelectOption &, unsigned int,
      bool, std::string, std::string, std::string, std::string);
  void addPreparedRaceTableHeader(unsigned int, MenuSelectOption &);
  void addPreparedRaceDetails(unsigned int, MenuSelectOption &, const Pointer<PreparedRace> &);
  unsigned int currentviewspan;
  unsigned int sitestartrow;
  int currentraces;
  int currenttransferjobs;
  std::string joblegendtext;
  std::string sitelegendtext;
  std::string preparelegendtext;
  std::string gotolegendtext;
  std::string activeracestext;
  std::string activejobstext;
  std::string numsitestext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  Ui * ui;
  MenuSelectSite mss;
  MenuSelectOption msop;
  MenuSelectOption mso;
  MenuSelectOption msot;
  bool gotomode;
};
