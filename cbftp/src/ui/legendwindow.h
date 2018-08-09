#pragma once

#include <list>
#include <memory>
#include <string>

#include "uiwindow.h"

class LegendPrinter;

struct _win_st;
typedef struct _win_st WINDOW;

class LegendWindow : public UIWindow {
public:
  LegendWindow(Ui *, WINDOW *, int, int);
  void redraw();
  void update();
  void setSplit(bool);
  void setMainLegendPrinter(std::shared_ptr<LegendPrinter> printer);
  void addTempLegendPrinter(std::shared_ptr<LegendPrinter> printer);
  void clearTempLegendPrinters();
private:
  int latestid;
  int latestcount;
  int staticcount;
  std::string latesttext;
  unsigned int offset;
  bool split;
  WINDOW * window;
  std::shared_ptr<LegendPrinter> mainlegendprinter;
  std::list<std::shared_ptr<LegendPrinter> > templegendprinters;
};
