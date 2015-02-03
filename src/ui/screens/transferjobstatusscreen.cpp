#include "transferjobstatusscreen.h"

#include "../ui.h"
#include "../resizableelement.h"
#include "../menuselectoptionelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionnumarrow.h"

#include "../../globalcontext.h"
#include "../../engine.h"
#include "../../transferstatus.h"
#include "../../transferjob.h"
#include "../../sitelogic.h"
#include "../../site.h"

extern GlobalContext * global;
TransferJobStatusScreen::TransferJobStatusScreen(Ui * ui) {
  this->ui = ui;
}

void TransferJobStatusScreen::initialize(unsigned int row, unsigned int col, std::string filename) {
  defaultlegendtext = "[c/Esc] Return - [Enter] Modify";
  currentlegendtext = defaultlegendtext;
  this->filename = filename;
  transferjob = global->getEngine()->getTransferJob(filename);
  autoupdate = true;
  mso.clear();
  mso.addIntArrow(3, 40, "slots", "Slots:", transferjob->maxSlots(), 1, transferjob->maxPossibleSlots());
  mso.enterFocusFrom(0);
  init(row, col);
}

void TransferJobStatusScreen::redraw() {
  ui->erase();
  table.clear();
  progressmap.clear();
  int y = 1;
  ui->printStr(y, 1, "Started: " + transferjob->timeStarted());
  ui->printStr(y, 20, "Type: " + transferjob->typeString());
  std::string route;
  switch (transferjob->getType()) {
    case TRANSFERJOB_DOWNLOAD:
    case TRANSFERJOB_DOWNLOAD_FILE:
      route = transferjob->getSrc()->getSite()->getName() + " -> /\\";
      break;
    case TRANSFERJOB_UPLOAD:
    case TRANSFERJOB_UPLOAD_FILE:
      route = "/\\ -> " + transferjob->getDst()->getSite()->getName();
      break;
    case TRANSFERJOB_FXP:
    case TRANSFERJOB_FXP_FILE:
      route = transferjob->getSrc()->getSite()->getName() + " -> " +
      transferjob->getDst()->getSite()->getName();
      break;
  }
  ui->printStr(y, 38, "Route: " + route);
  ui->printStr(y, 60, std::string("Status: ") + (transferjob->isDone() ? "Completed" : "In progress"));
  y++;
  ui->printStr(y, 1, "Size: " + GlobalContext::parseSize(transferjob->sizeProgress()) +
      " / " + GlobalContext::parseSize(transferjob->totalSize()));
  ui->printStr(y, 35, "Speed: " + GlobalContext::parseSize(transferjob->getSpeed() * SIZEPOWER) + "/s");
  ui->printStr(y, 60, "Files: " + global->int2Str(transferjob->filesProgress()) + "/" +
      global->int2Str(transferjob->filesTotal()));
  y++;
  ui->printStr(y, 1, "Time spent: " + GlobalContext::simpleTimeFormat(transferjob->timeSpent()));
  ui->printStr(y, 21, "Remaining: " + GlobalContext::simpleTimeFormat(transferjob->timeRemaining()));
  int progresspercent = transferjob->getProgress();
  std::string progress = "....................";
  int charswithhighlight = progress.length() * progresspercent / 100;
  ui->printStr(y, 53, "[");
  ui->printStr(y, 54, progress.substr(0, charswithhighlight), true);
  ui->printStr(y, 54 + charswithhighlight, progress.substr(charswithhighlight));
  ui->printStr(y, 54 + progress.length(), "] " + global->int2Str(transferjob->getProgress()) + "%");
  y = y + 2;
  addTransferDetails(y++, "USE", "TRANSFERRED", "FILENAME", "LEFT", "SPEED", "DONE", 0);
  for (std::list<Pointer<TransferStatus> >::const_iterator it = transferjob->transfersBegin(); it != transferjob->transfersEnd(); it++) {
    addTransferDetails(y++, *it);
  }
  for (std::map<std::string, unsigned long long int>::const_iterator it = transferjob->pendingTransfersBegin(); it != transferjob->pendingTransfersEnd(); it++) {
    addTransferDetails(y++, "-", GlobalContext::parseSize(0) + " / " + GlobalContext::parseSize(it->second),
        it->first, "-", "-", "wait", 0);
  }
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    ResizableElement * re = (ResizableElement *) table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      if (re->getIdentifier() == "filename") {
        int progresspercent = 0;
        std::map<MenuSelectOptionElement *, int>::iterator it = progressmap.find(re);
        if (it != progressmap.end()) {
          progresspercent = it->second;
        }
        std::string labeltext = re->getLabelText();
        int charswithhighlight = labeltext.length() * progresspercent / 100;
        ui->printStr(re->getRow(), re->getCol(), labeltext.substr(0, charswithhighlight), true);
        ui->printStr(re->getRow(), re->getCol() + charswithhighlight, labeltext.substr(charswithhighlight));
      }
      else {
        ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
      }
    }
  }
}

void TransferJobStatusScreen::update() {
  redraw();
}

void TransferJobStatusScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      if (activeelement->getIdentifier() == "slots") {
        int slots = ((MenuSelectOptionNumArrow *)activeelement)->getData();
        transferjob->setSlots(slots);
        switch (transferjob->getType()) {
          case TRANSFERJOB_DOWNLOAD:
          case TRANSFERJOB_FXP:
            transferjob->getSrc()->haveConnected(slots);
            break;
          case TRANSFERJOB_UPLOAD:
            transferjob->getDst()->haveConnected(slots);
            break;
        }
      }
      return;
    }
    activeelement->inputChar(ch);
    ui->update();
    return;
  }
  switch (ch) {
    case 'c':
    case 27: // esc
      ui->returnToLast();
      break;
    case 10:
    {
      bool activation = mso.activateSelected();
      if (activation) {
        active = true;
        activeelement = mso.getElement(mso.getSelectionPointer());
        currentlegendtext = activeelement->getLegendText();
        ui->update();
        ui->setLegend();
      }
      break;
    }
  }
}

std::string TransferJobStatusScreen::getLegendText() const {
  return currentlegendtext;
}

std::string TransferJobStatusScreen::getInfoLabel() const {
  return "TRANSFER JOB STATUS: " + filename;
}

void TransferJobStatusScreen::addTransferDetails(unsigned int y, Pointer<TransferStatus> ts) {
  std::string route = ts->getSource() + " -> " + ts->getTarget();
  std::string speed = GlobalContext::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  std::string timespent = GlobalContext::simpleTimeFormat(ts->getTimeSpent());
  int progresspercent = ts->getProgress();
  std::string progress;
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS:
      progress = global->int2Str(progresspercent) + "%";
      break;
    case TRANSFERSTATUS_STATE_FAILED:
      progress = "fail";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      progress = "done";
      break;
  }
  std::string timeremaining = GlobalContext::simpleTimeFormat(ts->getTimeRemaining());
  std::string transferred = GlobalContext::parseSize(ts->targetSize()) + " / " +
      GlobalContext::parseSize(ts->sourceSize());
  std::string path = ts->getSourcePath() + " -> " + ts->getTargetPath();
  std::string subpathfile = transferjob->findSubPath(ts) + ts->getFile();
  addTransferDetails(y, timespent, transferred, subpathfile, timeremaining,
                     speed, progress, progresspercent);
}

void TransferJobStatusScreen::addTransferDetails(unsigned int y, std::string timespent,
    std::string transferred, std::string file, std::string timeremaining,
    std::string speed, std::string progress, int progresspercent) {
  MenuSelectAdjustableLine * msal = table.addAdjustableLine();
  MenuSelectOptionTextButton * msotb;
  msotb = table.addTextButtonNoContent(y, 1, "timespent", timespent);
  msal->addElement((ResizableElement *)msotb, 9, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 10, "transferred", transferred);
  msal->addElement((ResizableElement *)msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 10, "filename", file);
  progressmap[(MenuSelectOptionElement *)msotb] = progresspercent;
  msal->addElement((ResizableElement *)msotb, 2, RESIZE_WITHLAST3, true);
  msotb = table.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msal->addElement((ResizableElement *)msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 40, "speed", speed);
  msal->addElement((ResizableElement *)msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 50, "progress", progress);
  msal->addElement((ResizableElement *)msotb, 7, RESIZE_REMOVE);
}
