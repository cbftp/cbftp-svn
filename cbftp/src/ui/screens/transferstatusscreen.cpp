#include "transferstatusscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"

#include "../../transferstatus.h"
#include "../../util.h"

TransferStatusScreen::TransferStatusScreen(Ui * ui) {
  this->ui = ui;
}

TransferStatusScreen::~TransferStatusScreen() {

}

void TransferStatusScreen::initialize(unsigned int row, unsigned int col, Pointer<TransferStatus> ts) {
  this->ts = ts;
  autoupdate = true;
  init(row, col);
}

void TransferStatusScreen::redraw() {
  ui->erase();
  unsigned int y = 1;
  int type = ts->getType();
  ui->printStr(y, 1, "Started: " + ts->getTimestamp());
  std::string route = ts->getSource() + " -> " + ts->getTarget();
  ui->printStr(y, 20, "Route: " + route);
  ++y;
  ui->printStr(y, 1, "File name: " + ts->getFile());
  ++y;
  unsigned int x = 1;
  if (type != TRANSFERSTATUS_TYPE_UPLOAD) {
    ui->printStr(y, x, "Source slot: " + util::int2Str(ts->getSourceSlot()));
    x += 18;
  }
  if (type != TRANSFERSTATUS_TYPE_DOWNLOAD) {
    ui->printStr(y, x, "Target slot: " + util::int2Str(ts->getTargetSlot()));
    x += 18;
  }
  bool ssl = ts->getSSL();
  ui->printStr(y, x, std::string("SSL: ") + (ssl ? "Yes" : "No"));
  x += 12;
  if (type != TRANSFERSTATUS_TYPE_FXP && ssl) {
    ui->printStr(y, x, "Cipher: " + ts->getCipher());
  }
  ++y;
  ui->printStr(y, 1, "Source path: " + ts->getSourcePath());
  ++y;
  ui->printStr(y, 1, "Target path: " + ts->getTargetPath());
  ++y;
  TransferDetails td = TransfersScreen::formatTransferDetails(ts);
  std::string passive;
  bool defaultactive = ts->getDefaultActive();
  if (type == TRANSFERSTATUS_TYPE_FXP) {
    passive = defaultactive ? ts->getSource() : ts->getTarget();
  }
  else {
    if (defaultactive) {
     passive = (type == TRANSFERSTATUS_TYPE_DOWNLOAD ? ts->getSource() : ts->getTarget());
    }
    else {
     passive = "local";
    }
  }
  ui->printStr(y, 1, "Passive: " + passive + "     Passive address: " + ts->getPassiveAddress());
  ++y;
  ui->printStr(y, 1, "Size: " + td.transferred);
  ui->printStr(y, 35, "Speed: " + td.speed);
  std::string progress;
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS:
      progress = "In progress";
      break;
    case TRANSFERSTATUS_STATE_FAILED:
      progress = "Failed";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      progress = "Completed";
      break;
    case TRANSFERSTATUS_STATE_DUPE:
      progress = "Failed (Dupe)";
      break;
  }
  ui->printStr(y, 57, "Status: " + progress);
  ++y;
  ui->printStr(y, 1, "Time spent: " + td.timespent);
  ui->printStr(y, 21, "Remaining: " + td.timeremaining);

  int progresspercent = ts->getProgress();
  progress = "....................";
  int charswithhighlight = progress.length() * progresspercent / 100;
  ui->printStr(y, 47, "[");
  ui->printStr(y, 48, progress.substr(0, charswithhighlight), true);
  ui->printStr(y, 48 + charswithhighlight, progress.substr(charswithhighlight));
  ui->printStr(y, 48 + progress.length(), "] " + util::int2Str(progresspercent) + "%");
  ++y;
  ++y;
  for (std::list<std::string>::const_iterator it = ts->getLogLines().begin(); it != ts->getLogLines().end() && y < row; it++) {
    ui->printStr(y++, 1, *it);
  }
}

void TransferStatusScreen::update() {
  redraw();
}

bool TransferStatusScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case 'c':
    case 10:
    case 27: // esc
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransferStatusScreen::getLegendText() const {
  return "[c/Enter/Esc] Return";
}

std::string TransferStatusScreen::getInfoLabel() const {
  return "TRANSFER STATUS: " + ts->getFile() + " " + ts->getSource() + " -> " + ts->getTarget();
}
