#pragma once

#include <list>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class UICommunicator;
class MenuSelectOptionElement;
class Site;

class NewRaceScreen : public UIWindow {
public:
  NewRaceScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
private:
  void populateSiteList();
  bool startRace();
  Site * startsite;
  std::string getSectionButtonText(MenuSelectOptionElement *);
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool toggleall;
  bool sectionupdate;
  std::string section;
  MenuSelectOptionElement * activeelement;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  MenuSelectOption msos;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
  std::list<std::string> sections;
  std::string release;
  std::string infotext;
};
