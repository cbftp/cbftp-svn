#include "transfersfilterscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"

TransfersFilterScreen::TransfersFilterScreen(Ui * ui) {
  this->ui = ui;
}

TransfersFilterScreen::~TransfersFilterScreen() {

}

void TransfersFilterScreen::initialize(unsigned int row, unsigned int col) {
  mso.reset();
  mso.enterFocusFrom(0);
  init(row, col);
}

void TransfersFilterScreen::redraw() {
  ui->erase();
}

void TransfersFilterScreen::update() {

}

bool TransfersFilterScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case 'd':
    case 'f': {
      TransferFilteringParameters tfp;
      ui->returnTransferFilters(tfp);
      return true;
    }
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransfersFilterScreen::getLegendText() const {
  return "moo";
}

std::string TransfersFilterScreen::getInfoLabel() const {
  return "TRANSFERS FILTERING";
}
