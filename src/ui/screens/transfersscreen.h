#pragma once

#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;

class TransfersScreen : public UIWindow {
public:
  TransfersScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  void addTransferDetails(unsigned int, TransferStatus *);
  std::map<MenuSelectOptionElement *, int> progressmap;
  TransferManager * tm;
  MenuSelectOption table;
};
