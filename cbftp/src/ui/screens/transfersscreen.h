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
  static void addTransferTableHeader(unsigned int, MenuSelectOption &);
private:
  static Pointer<MenuSelectOptionElement> addTransferTableRow(unsigned int, MenuSelectOption &, bool,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &);
  static Pointer<MenuSelectOptionElement> addTransferDetails(unsigned int, MenuSelectOption &, Pointer<TransferStatus>);
  std::map<Pointer<MenuSelectOptionElement>, int> progressmap;
  TransferManager * tm;
  MenuSelectOption table;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool hascontents;
};
