#pragma once

#include <list>
#include <string>

#include "../core/pointer.h"

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
  void setMainLegendPrinter(Pointer<LegendPrinter> printer);
  void addTempLegendPrinter(Pointer<LegendPrinter> printer);
  void clearTempLegendPrinters();
private:
  int latestid;
  int latestcount;
  int staticcount;
  std::string latesttext;
  unsigned int offset;
  bool split;
  WINDOW * window;
  Pointer<LegendPrinter> mainlegendprinter;
  std::list<Pointer<LegendPrinter> > templegendprinters;
};
