#pragma once

#include <map>
#include <list>

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

struct TransferFilteringParameters {
  TransferFilteringParameters() :
    showstatusinprogress(true),
    showstatusdone(true), showstatusfail(true),
    usepathfilter(false), usesourcesitefilter(false),
    usetargetsitefilter(false) { }
  bool showstatusinprogress;
  bool showstatusdone;
  bool showstatusfail;
  bool showstatusdupe;
  bool usepathfilter;
  std::list<std::string> pathfilters;
  bool usesourcesitefilter;
  std::list<std::string> sourcesitefilters;
  bool usetargetsitefilter;
  std::list<std::string> targetsitefilters;
};

class TransfersScreen : public UIWindow {
public:
  TransfersScreen(Ui *);
  ~TransfersScreen();
  void initialize(unsigned int, unsigned int);
  void initialize(unsigned int, unsigned int, const TransferFilteringParameters &);
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
  bool filtering;
  TransferFilteringParameters tfp;
};
