#pragma once

#include <string>

#include "../core/pointer.h"

#include "legendprinter.h"
#include "menuselectoption.h"

class Race;
class Ui;

class LegendPrinterSpreadJob : public LegendPrinter {
public:
  LegendPrinterSpreadJob(Ui * ui, unsigned int id);
  bool print();
private:
  Ui * ui;
  Pointer<Race> race;
  MenuSelectOption mso;
  int jobfinishedprintcount;
};
