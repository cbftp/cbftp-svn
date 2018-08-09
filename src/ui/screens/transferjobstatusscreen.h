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
  void initialize(unsigned int, unsigned int, unsigned int);
  void redraw();
  void update();
  void command(const std::string &, const std::string &);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  static std::string getRoute(std::shared_ptr<TransferJob>);
private:
  void addTransferDetails(unsigned int, std::shared_ptr<TransferStatus>);
  void addTransferDetails(unsigned int, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, int);
  std::shared_ptr<TransferJob> transferjob;
  MenuSelectOption table;
  MenuSelectOption mso;
  std::map<std::shared_ptr<MenuSelectOptionElement>, int> progressmap;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  std::string defaultlegendtext;
  std::string currentlegendtext;
  std::string abortedlegendtext;
};
