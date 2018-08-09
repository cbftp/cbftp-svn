#pragma once

#include <map>
#include <memory>
#include <string>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Ui;
class TransferStatus;
class MenuSelectOptionElement;
class TransferStatus;

class TransferStatusScreen : public UIWindow {
public:
  TransferStatusScreen(Ui *);
  ~TransferStatusScreen();
  void initialize(unsigned int, unsigned int, std::shared_ptr<TransferStatus>);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  static void abortTransfer(std::shared_ptr<TransferStatus> ts);
private:
  std::shared_ptr<TransferStatus> ts;
  MenuSelectOption table;
  MenuSelectOption mso;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  std::string defaultlegendtext;
  std::string currentlegendtext;
  std::string abortedlegendtext;
  unsigned int currentviewspan;
  unsigned int logstart;
  bool firstdraw;
};
