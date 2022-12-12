#pragma once

#include <map>
#include <string>

#include <memory>

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
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  static void addJobTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addJobDetails(unsigned int, MenuSelectOption &, std::shared_ptr<Race>);
  static void addJobDetails(unsigned int, MenuSelectOption &, std::shared_ptr<TransferJob>);
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
