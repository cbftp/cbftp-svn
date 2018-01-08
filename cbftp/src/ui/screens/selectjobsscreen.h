#pragma once

#include <map>
#include <string>

#include "../../core/pointer.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class CommandOwner;
class TransferJob;
class Race;

enum JobType {
  JOBTYPE_TRANSFERJOB,
  JOBTYPE_SPREADJOB
};

class SelectJobsScreen : public UIWindow {
public:
  SelectJobsScreen(Ui *);
  void initialize(unsigned int, unsigned int, JobType);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  static void addJobTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addJobDetails(unsigned int, MenuSelectOption &, Pointer<Race>);
  static void addJobDetails(unsigned int, MenuSelectOption &, Pointer<TransferJob>);
private:
  static void addTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &, const std::string &);
  MenuSelectOption table;
  bool hascontents;
  unsigned int currentviewspan;
  unsigned int ypos;
  int numselected;
  unsigned int totallistsize;
  JobType type;
};
