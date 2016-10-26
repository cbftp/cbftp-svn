#pragma once

#include <list>

#include "../../core/pointer.h"
#include "../../site.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../menusection.h"

class FocusableArea;
class Site;
class MenuSelectOptionElement;

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(Ui * ui);
  ~EditSiteScreen();
  void initialize(unsigned int, unsigned int, const std::string &, const std::string &);
  void update();
  void redraw();
  void command(const std::string &, const std::string &);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void fillPreselectionList(const std::string &, std::list<Pointer<Site> > *) const;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  MenuSection ms;
  Pointer<Site> site;
  Site modsite;
  std::string operation;
};
