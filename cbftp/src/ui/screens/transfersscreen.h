#pragma once

#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;

class TransfersScreen : public UIWindow {
public:
  TransfersScreen(Ui *);
  ~TransfersScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void addTransferDetails(unsigned int, MenuSelectOption &, Pointer<TransferStatus>);
  std::map<Pointer<MenuSelectOptionElement>, int> progressmap;
  TransferManager * tm;
  MenuSelectOption table;
};
