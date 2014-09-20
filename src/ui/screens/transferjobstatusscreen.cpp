#include "transferjobstatusscreen.h"

#include "../ui.h"

#include "../../globalcontext.h"
#include "../../engine.h"

extern GlobalContext * global;
TransferJobStatusScreen::TransferJobStatusScreen(Ui * ui) {
  this->ui = ui;
}

void TransferJobStatusScreen::initialize(unsigned int row, unsigned int col, std::string filename) {
  this->filename = filename;
  transferjob = global->getEngine()->getTransferJob(filename);
  init(row, col);
}

void TransferJobStatusScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "placeholder");
}

void TransferJobStatusScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case 'c':
    case 27: // esc
      ui->returnToLast();
      break;
  }
}

std::string TransferJobStatusScreen::getLegendText() const {
  return "[c/Esc] Return";
}

std::string TransferJobStatusScreen::getInfoLabel() const {
  return "TRANSFER JOB STATUS: " + filename;
}
