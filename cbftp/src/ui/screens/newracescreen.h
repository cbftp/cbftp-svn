#pragma once

#include <list>
#include <memory>
#include <utility>


#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;
class Site;
class Race;

class NewRaceScreen : public UIWindow {
public:
  NewRaceScreen(Ui *);
  ~NewRaceScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & site, const std::list<std::string> & sections, const std::list<std::pair<std::string, bool> > & items);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  void populateSiteList();
  std::shared_ptr<Race> startRace(bool addtemplegend);
  std::shared_ptr<Site> startsite;
  std::string getSectionButtonText(std::shared_ptr<MenuSelectOptionElement>) const;
  bool active;
  bool toggleall;
  std::string section;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  MenuSelectOption msos;
  MenuSelectOption mso;
  std::list<std::string> sections;
  std::list<std::pair<std::string, bool> > items;
  std::string infotext;
  std::list<std::pair<std::string, bool> > tempsites;
  std::shared_ptr<MenuSelectOptionTextArrow> msota;
};
