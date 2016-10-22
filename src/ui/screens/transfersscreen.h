#pragma once

#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;

struct TransferDetails {
  std::string route;
  std::string timespent;
  std::string progress;
  std::string timeremaining;
  std::string speed;
  std::string transferred;
  std::string timestamp;
  std::string path;
};

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
  static TransferDetails formatTransferDetails(Pointer<TransferStatus> &);
private:
  static void addTransferTableRow(unsigned int, MenuSelectOption &, bool,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &,
      int);
  static void addTransferDetails(unsigned int, MenuSelectOption &, Pointer<TransferStatus>, int);
  std::map<int, Pointer<TransferStatus> > statusmap;
  TransferManager * tm;
  MenuSelectOption table;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool hascontents;
  int nextid;
};
