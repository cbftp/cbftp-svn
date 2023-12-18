#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "../../core/eventreceiver.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class Race;

struct SpreadJobsFilteringParameters {
  SpreadJobsFilteringParameters() :
    usejobnamefilter(false), usesitefilter(false),
    usestatusfilter(false), showstatusinprogress(false),
    showstatusdone(false), showstatusaborted(false), showstatustimeout(false) { }
  bool usejobnamefilter;
  bool usesitefilter;
  std::list<std::string> anysitefilters;
  std::list<std::string> allsitefilters;
  std::string jobnamefilter;
  bool usestatusfilter;
  bool showstatusinprogress;
  bool showstatusdone;
  bool showstatusaborted;
  bool showstatustimeout;
};

class AllRacesScreen : public UIWindow, protected Core::EventReceiver {
public:
  AllRacesScreen(Ui *);
  ~AllRacesScreen();
  void initialize(unsigned int row, unsigned int col);
  void initializeFilterSite(unsigned int row, unsigned int col, const std::string& site);
  void initialize(unsigned int row, unsigned int col, const SpreadJobsFilteringParameters& sjfp);
  void redraw() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  std::string getLegendText() const override;
  static void addRaceTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addRaceDetails(unsigned int, MenuSelectOption &, std::shared_ptr<Race>);
private:
  static void addRaceTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
  bool showsWhileFiltered(const std::shared_ptr<Race>& race) const;
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
  std::shared_ptr<Race> abortrace;
  std::shared_ptr<Race> abortdeleteraceinc;
  std::shared_ptr<Race> abortdeleteraceall;
  SpreadJobsFilteringParameters sjfp;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  std::string gotomodestring;
};
