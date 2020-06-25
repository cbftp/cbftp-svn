#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../transferpairing.h"

class Ui;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextField;
class MenuSelectOptionTextArrow;
class MenuSelectOptionElement;

class TransferPairingScreen : public UIWindow {
public:
  TransferPairingScreen(Ui* ui);
  ~TransferPairingScreen();
  void initialize(unsigned int row, unsigned int col, TransferPairing* transferpairing);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void initialize();
  //void addPatternLine(int y);
  void keyUp();
  void keyDown();
  TransferPairing* transferpairing;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption table;
  unsigned int currentviewspan;
  int temphighlightline;
};
