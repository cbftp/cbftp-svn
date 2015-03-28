#pragma once

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../menuselectoption.h"

class FocusableArea;
class Ui;

class MainScreen : public UIWindow {
public:
  MainScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void command(std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  unsigned int currentviewspan;
  unsigned int sitestartrow;
  int currentraces;
  int currenttransferjobs;
  std::string baselegendtext;
  std::string siteextralegendtext;
  std::string gotolegendtext;
  std::string activeracestext;
  std::string activejobstext;
  std::string numsitestext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  Ui * ui;
  MenuSelectSite mss;
  MenuSelectOption mso;
  MenuSelectOption msot;
  bool gotomode;
};
