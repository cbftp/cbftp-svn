#pragma once

#include "../../site.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../menusection.h"

class FocusableArea;
class MenuSelectOptionElement;

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(Ui * ui);
  void initialize(unsigned int, unsigned int, std::string, std::string);
  void update();
  void redraw();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void fillPreselectionList(std::string, std::list<Site *> *) const;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  MenuSection ms;
  Site * site;
  Site modsite;
  std::string operation;
};
