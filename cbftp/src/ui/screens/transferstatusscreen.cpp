#include "transferstatusscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../misc.h"

#include "../../transferstatus.h"
#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"

TransferStatusScreen::TransferStatusScreen(Ui* ui) : UIWindow(ui, "TransferStatusScreen") {
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind('B', KEYACTION_ABORT, "Abort transfer");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
}

TransferStatusScreen::~TransferStatusScreen() {

}

void TransferStatusScreen::initialize(unsigned int row, unsigned int col, std::shared_ptr<TransferStatus> ts) {
  this->ts = ts;
  autoupdate = true;
  firstdraw = true;
  currentviewspan = 0;
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
    ui->printStr(y, x, "Source slot: " + std::to_string(ts->getSourceSlot()));
    x += 18;
  }
  if (type != TRANSFERSTATUS_TYPE_DOWNLOAD) {
    ui->printStr(y, x, "Target slot: " + std::to_string(ts->getTargetSlot()));
    x += 18;
  }
  bool ssl = ts->getSSL();
  ui->printStr(y, x, std::string("TLS: ") + (ssl ? "Yes" : "No"));
  if (type != TRANSFERSTATUS_TYPE_FXP && ssl) {
    ++y;
    ui->printStr(y, 1, std::string("TLS session reused: ") + (ts->getSSLSessionReused() ? "Yes" : "No"));
    ui->printStr(y, 30, "Cipher: " + ts->getCipher());

  }
  ++y;
  ui->printStr(y, 1, "Source path: " + ts->getSourcePath().toString());
  ++y;
  ui->printStr(y, 1, "Target path: " + ts->getTargetPath().toString());
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
    case TRANSFERSTATUS_STATE_ABORTED:
      progress = "Aborted";
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
  ui->printStr(y, 48 + progress.length(), "] " + std::to_string(progresspercent) + "%");
  ++y;
  ++y;
  logstart = y;
  unsigned int totalspan = ts->getLogLines().size();
  unsigned int i = 0;
  if (row > logstart && row - logstart < totalspan) {
    if (firstdraw || currentviewspan > totalspan - (row - logstart)) {
      currentviewspan = totalspan - (row - logstart);
    }
  }
  if (firstdraw) {
    firstdraw = false;
  }
  for (std::list<std::string>::const_iterator it = ts->getLogLines().begin(); it != ts->getLogLines().end() && y < row; it++, i++) {
    if (i >= currentviewspan) {
      ui->printStr(y++, 1, *it);
    }
  }

  printSlider(ui, row, logstart, col - 1, totalspan, currentviewspan);
}

void TransferStatusScreen::update() {
  redraw();
}

bool TransferStatusScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  unsigned int totalspan = ts->getLogLines().size();
  unsigned int rowspan = row - logstart;
  switch (action) {
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_ABORT:
      abortTransfer(ts);
      return true;
    case KEYACTION_UP:
      if (currentviewspan < 2) {
        currentviewspan = 0;
      }
      else {
        currentviewspan -= 2;
      }
      ui->update();
      return true;
    case KEYACTION_DOWN:
      if (rowspan < totalspan) {
        if (currentviewspan < totalspan - rowspan) {
          currentviewspan += 2;
        }
        if (currentviewspan > totalspan - rowspan) {
          currentviewspan = totalspan - rowspan;
        }
      }
      if (rowspan >= totalspan) {
        currentviewspan = 0;
      }
      ui->update();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      if (currentviewspan < rowspan * 0.5) {
        currentviewspan = 0;
      }
      else {
        currentviewspan -= rowspan * 0.5;
      }
      ui->update();
      return true;
    case KEYACTION_NEXT_PAGE:
      if (rowspan * 1.5 < totalspan && currentviewspan < totalspan - rowspan * 1.5) {
        currentviewspan += rowspan * 0.5;
      }
      else if (totalspan > rowspan) {
        currentviewspan = totalspan - rowspan;
      }
      ui->update();
      return true;
    case KEYACTION_TOP:
      currentviewspan = 0;
      ui->update();
      return true;
    case KEYACTION_BOTTOM:
      if (totalspan > rowspan) {
        currentviewspan = totalspan - rowspan;
      }
      ui->update();
      return true;
  }
  return false;
}

void TransferStatusScreen::abortTransfer(std::shared_ptr<TransferStatus> ts) {
  if (ts->getState() == TRANSFERSTATUS_STATE_IN_PROGRESS) {
    int type = ts->getType();
    if (type == TRANSFERSTATUS_TYPE_DOWNLOAD || type == TRANSFERSTATUS_TYPE_FXP) {
      global->getSiteLogicManager()->getSiteLogic(ts->getSource())->disconnectConn(ts->getSourceSlot());
    }
    if (type == TRANSFERSTATUS_TYPE_UPLOAD || type == TRANSFERSTATUS_TYPE_FXP) {
      global->getSiteLogicManager()->getSiteLogic(ts->getTarget())->disconnectConn(ts->getTargetSlot());
    }
    ts->setAborted();
  }
}

std::string TransferStatusScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string TransferStatusScreen::getInfoLabel() const {
  return "TRANSFER STATUS: " + ts->getFile() + " " + ts->getSource() + " -> " + ts->getTarget();
}
