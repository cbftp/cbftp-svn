#pragma once

#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../pointer.h"

class AllTransferJobsScreen : public UIWindow {
public:
  AllTransferJobsScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  MenuSelectOption table;
};
