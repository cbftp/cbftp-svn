#pragma once

#include <list>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class MenuSelectOptionElement;
class Site;

class NewRaceScreen : public UIWindow {
public:
  NewRaceScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, std::string, std::string);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void populateSiteList();
  bool startRace();
  Site * startsite;
  std::string getSectionButtonText(MenuSelectOptionElement *) const;
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
  std::list<std::string> sections;
  std::string release;
  std::string infotext;
};
