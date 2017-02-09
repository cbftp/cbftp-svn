#pragma once

#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;

class TransfersFilterScreen : public UIWindow {
public:
  TransfersFilterScreen(Ui *);
  ~TransfersFilterScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  MenuSelectOption mso;
};
