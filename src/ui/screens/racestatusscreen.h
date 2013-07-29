#pragma once

#include <vector>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../ftpconn.h"
#include "../../engine.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../termint.h"

extern GlobalContext * global;

class RaceStatusScreen : public UIWindow {
public:
  RaceStatusScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  Race * race;
  bool spaceous;
  unsigned int currnumsubpaths;
  unsigned int currguessedsize;
  std::string release;
  std::string sitestr;
  std::list<std::string> subpaths;
  UICommunicator * uicommunicator;
  std::map<std::string, int> filetagpos;
};
