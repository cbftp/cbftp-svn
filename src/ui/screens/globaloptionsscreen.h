#pragma once

#include <ncurses.h>
#include <list>

#include "../../globalcontext.h"
#include "../../remotecommandhandler.h"
#include "../../sitemanager.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../menuselectoption.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"
#include "../termint.h"

extern GlobalContext * global;

class GlobalOptionsScreen : public UIWindow {
public:
  GlobalOptionsScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string getSectionButtonText(MenuSelectOptionElement *);
  MenuSelectOption mso;
  RemoteCommandHandler * rch;
  SiteManager * sm;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  UICommunicator * uicommunicator;
};
