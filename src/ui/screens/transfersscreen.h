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
    usejobfilter(false), usesitefilter(false), usefilenamefilter(false),
    usestatusfilter(false), showstatusinprogress(false),
    showstatusdone(false), showstatusfail(false), showstatusdupe(false) { }
  bool usejobfilter;
  std::list<std::string> spreadjobsfilter;
  std::list<std::string> transferjobsfilter;
  bool usesitefilter;
  std::list<std::string> sourcesitefilters;
  std::list<std::string> targetsitefilters;
  std::list<std::string> anydirectionsitefilters;
  bool usefilenamefilter;
  std::string filenamefilter;
  bool usestatusfilter;
  bool showstatusinprogress;
  bool showstatusdone;
  bool showstatusfail;
  bool showstatusdupe;

};

class TransfersScreen : public UIWindow {
public:
  TransfersScreen(Ui *);
  ~TransfersScreen();
  void initialize(unsigned int, unsigned int);
  void initializeFilterSite(unsigned int, unsigned int, const std::string &);
  void initializeFilterSpreadJob(unsigned int, unsigned int, const std::string &);
  void initializeFilterTransferJob(unsigned int, unsigned int, const std::string &);
  void initialize(unsigned int, unsigned int, const TransferFilteringParameters &);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  static void addTransferTableHeader(unsigned int, MenuSelectOption &);
  static TransferDetails formatTransferDetails(Pointer<TransferStatus> &);
private:
  static void addTransferTableRow(unsigned int, MenuSelectOption &, bool,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &,
      int);
  static void addTransferDetails(unsigned int, MenuSelectOption &, Pointer<TransferStatus>, int);
  void addFilterFinishedTransfers();
  bool showsWhileFiltered(const Pointer<TransferStatus> &) const;
  std::map<int, Pointer<TransferStatus> > statusmap;
  TransferManager * tm;
  MenuSelectOption table;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool hascontents;
  int nextid;
  bool filtering;
  TransferFilteringParameters tfp;
  int numfinishedfiltered;
  std::list<Pointer<TransferStatus> > finishedfilteredtransfers;
};
