#pragma once

#include <string>
#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"

class Ui;
class TransferStatus;
class MenuSelectOptionElement;
class TransferStatus;

class TransferStatusScreen : public UIWindow {
public:
  TransferStatusScreen(Ui *);
  ~TransferStatusScreen();
  void initialize(unsigned int, unsigned int, Pointer<TransferStatus>);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  Pointer<TransferStatus> ts;
  MenuSelectOption table;
  MenuSelectOption mso;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  std::string defaultlegendtext;
  std::string currentlegendtext;
  std::string abortedlegendtext;
};
