#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "../../core/eventreceiver.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class TransferJob;

struct TransferJobsFilteringParameters {
  TransferJobsFilteringParameters() :
    usejobnamefilter(false), usesitefilter(false),
    usestatusfilter(false), showstatusqueued(false), showstatusinprogress(false),
    showstatusdone(false), showstatusaborted(false) { }
  bool usejobnamefilter;
  bool usesitefilter;
  std::list<std::string> sourcesitefilters;
  std::list<std::string> targetsitefilters;
  std::list<std::string> anydirectionsitefilters;
  std::string jobnamefilter;
  bool usestatusfilter;
  bool showstatusqueued;
  bool showstatusinprogress;
  bool showstatusdone;
  bool showstatusaborted;
};

class AllTransferJobsScreen : public UIWindow, protected Core::EventReceiver {
public:
  AllTransferJobsScreen(Ui *);
  ~AllTransferJobsScreen();
  void initialize(unsigned int row, unsigned int col);
  void initializeFilterSite(unsigned int row, unsigned int col, const std::string& site);
  void initialize(unsigned int row, unsigned int col, const TransferJobsFilteringParameters& tjfp);
  void redraw() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  std::string getLegendText() const override;
  static void addJobTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addJobDetails(unsigned int, MenuSelectOption &, std::shared_ptr<TransferJob>);
private:
  static void addJobTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
  bool showsWhileFiltered(const std::shared_ptr<TransferJob>& tj) const;
  unsigned int totalListSize() const;
  void tick(int) override;
  void disableGotoMode();
  MenuSelectOption table;
  Engine * engine;
  bool hascontents;
  bool filtering;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool temphighlightline;
  std::shared_ptr<TransferJob> abortjob;
  TransferJobsFilteringParameters tjfp;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  std::string gotomodestring;
};
