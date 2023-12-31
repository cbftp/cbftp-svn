#pragma once

#include <map>
#include <memory>
#include <string>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class TransferJob;
class Ui;
class TransferStatus;
class MenuSelectOptionElement;

class TransferJobStatusScreen : public UIWindow {
public:
  TransferJobStatusScreen(Ui *);
  ~TransferJobStatusScreen();
  bool initialize(unsigned int, unsigned int, unsigned int);
  void redraw() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  static std::string getRoute(std::shared_ptr<TransferJob>);
private:
  void addTransferDetails(unsigned int, std::shared_ptr<TransferStatus>);
  void addTransferDetails(unsigned int, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, int);
  int getCurrentScope() const;
  std::shared_ptr<TransferJob> transferjob;
  MenuSelectOption table;
  MenuSelectOption mso;
  std::map<std::shared_ptr<MenuSelectOptionElement>, int> progressmap;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
};
