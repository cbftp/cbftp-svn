#pragma once

#include <string>

#include "../core/pointer.h"

#include "legendprinter.h"
#include "menuselectoption.h"

class TransferJob;
class Ui;

class LegendPrinterTransferJob : public LegendPrinter {
public:
  LegendPrinterTransferJob(Ui * ui, unsigned int id);
  bool print();
private:
  Ui * ui;
  Pointer<TransferJob> transferjob;
  MenuSelectOption mso;
  int jobfinishedprintcount;
};
