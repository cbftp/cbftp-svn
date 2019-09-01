#include "transferjobstatusscreen.h"

#include <unordered_map>

#include "transfersscreen.h"

#include "../ui.h"
#include "../resizableelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"

#include "../../transferjob.h"
#include "../../globalcontext.h"
#include "../../engine.h"
#include "../../transferstatus.h"
#include "../../transferjob.h"
#include "../../sitelogic.h"
#include "../../site.h"
#include "../../util.h"

TransferJobStatusScreen::TransferJobStatusScreen(Ui * ui) {
  this->ui = ui;
}

TransferJobStatusScreen::~TransferJobStatusScreen() {

}

void TransferJobStatusScreen::initialize(unsigned int row, unsigned int col, unsigned int id) {
  abortedlegendtext = "[c/Esc] Return";
  defaultlegendtext = abortedlegendtext + " - [Enter] Modify - A[B]ort transfer job - [t]ransfers";
  transferjob = global->getEngine()->getTransferJob(id);
  currentlegendtext = transferjob->getStatus() == TRANSFERJOB_ABORTED ? abortedlegendtext : defaultlegendtext;
  autoupdate = true;
  active = false;
  mso.clear();
  mso.addIntArrow(5, 40, "slots", "Slots:", transferjob->maxSlots(), 1, transferjob->maxPossibleSlots());
  mso.enterFocusFrom(0);
  init(row, col);
}

void TransferJobStatusScreen::redraw() {
  ui->erase();
  table.clear();
  progressmap.clear();
  int y = 1;
  bool running = transferjob->getStatus() == TRANSFERJOB_RUNNING;
  bool started = transferjob->getStatus() != TRANSFERJOB_QUEUED;
  ui->printStr(y, 1, "Queued: " + transferjob->timeQueued());
  if (started) {
    ui->printStr(y, 19, "Started: " + transferjob->timeStarted());
  }
  y++;
  ui->printStr(y, 1, "Type: " + transferjob->typeString());
  std::string route = getRoute(transferjob);
  ui->printStr(y, 19, "Route: " + route);
  std::string status;
  switch (transferjob->getStatus()) {
    case TRANSFERJOB_QUEUED:
      status = "Queued";
      break;
    case TRANSFERJOB_RUNNING:
      status = "In progress";
      break;
    case TRANSFERJOB_DONE:
      status = "Completed";
      break;
    case TRANSFERJOB_ABORTED:
      status = "Aborted";
      break;
  }
  ui->printStr(y, 53, std::string("Status: ") + status);
  y++;
  ui->printStr(y, 1, "Path: " + transferjob->getSrcPath().toString() + " -> " + transferjob->getDstPath().toString());
  ++y;
  ui->printStr(y, 1, "Size: " + util::parseSize(transferjob->sizeProgress()) +
      " / " + util::parseSize(transferjob->totalSize()));
  ui->printStr(y, 35, "Speed: " + (started ? util::parseSize(transferjob->getSpeed() * SIZEPOWER) + "/s" : "-"));
  ui->printStr(y, 60, "Files: " + std::to_string(transferjob->filesProgress()) + "/" +
      std::to_string(transferjob->filesTotal()));
  y++;
  ui->printStr(y, 1, "Time spent: " + util::simpleTimeFormat(transferjob->timeSpent()));
  ui->printStr(y, 21, "Remaining: " + (running ? util::simpleTimeFormat(transferjob->timeRemaining()) : "-"));
  int progresspercent = transferjob->getProgress();
  std::string progress = "....................";
  int charswithhighlight = progress.length() * progresspercent / 100;
  ui->printStr(y, 53, "[");
  ui->printStr(y, 54, progress.substr(0, charswithhighlight), true);
  ui->printStr(y, 54 + charswithhighlight, progress.substr(charswithhighlight));
  ui->printStr(y, 54 + progress.length(), "] " + std::to_string(progresspercent) + "%");
  y = y + 2;
  addTransferDetails(y++, "USE", "TRANSFERRED", "FILENAME", "LEFT", "SPEED", "DONE", 0);
  for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = transferjob->transfersBegin(); it != transferjob->transfersEnd(); it++) {
    addTransferDetails(y++, *it);
  }
  for (std::unordered_map<std::string, unsigned long long int>::const_iterator it = transferjob->pendingTransfersBegin(); it != transferjob->pendingTransfersEnd(); it++) {
    addTransferDetails(y++, "-", util::parseSize(0) + " / " + util::parseSize(it->second),
        it->first, "-", "-", "wait", 0);
  }
  table.adjustLines(col - 3);
  bool highlight;
  if (!transferjob->isDone()) {
    for (unsigned int i = 0; i < mso.size(); i++) {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
      highlight = false;
      if (mso.isFocused() && mso.getSelectionPointer() == i) {
        highlight = true;
      }
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
      ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    }
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      if (re->getIdentifier() == "transferred") {
        int progresspercent = 0;
        std::map<std::shared_ptr<MenuSelectOptionElement>, int>::iterator it = progressmap.find(re);
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

void TransferJobStatusScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    global->getEngine()->abortTransferJob(transferjob);
    currentlegendtext = abortedlegendtext;
    ui->setLegend();
  }
  ui->redraw();
}

bool TransferJobStatusScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      if (activeelement->getIdentifier() == "slots") {
        int slots = std::static_pointer_cast<MenuSelectOptionNumArrow>(activeelement)->getData();
        transferjob->setSlots(slots);
        int type = transferjob->getType();
        if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) {
          transferjob->getSrc()->haveConnectedActivate(slots);
        }
        if (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP) {
          transferjob->getDst()->haveConnectedActivate(slots);
        }
      }
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  switch (ch) {
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case 10:
      if (!transferjob->isDone()) {
        bool activation = mso.activateSelected();
        if (activation) {
          active = true;
          activeelement = mso.getElement(mso.getSelectionPointer());
          currentlegendtext = activeelement->getLegendText();
          ui->update();
          ui->setLegend();
        }
      }
      return true;
    case 'B':
      if (!transferjob->isDone()) {
        ui->goConfirmation("Do you really want to abort the transfer job " + transferjob->getName());
      }
      return true;
    case 't':
      ui->goTransfersFilterTransferJob(transferjob->getName());
      return true;
  }
  return false;
}

std::string TransferJobStatusScreen::getLegendText() const {
  return currentlegendtext;
}

std::string TransferJobStatusScreen::getInfoLabel() const {
  return "TRANSFER JOB STATUS: " + transferjob->getSrcFileName();
}

void TransferJobStatusScreen::addTransferDetails(unsigned int y, std::shared_ptr<TransferStatus> ts) {
  TransferDetails td = TransfersScreen::formatTransferDetails(ts);
  Path subpathfile = (ts->getSourcePath() - transferjob->getSrcPath()) / ts->getFile();
  addTransferDetails(y, td.timespent, td.transferred, subpathfile.toString(), td.timeremaining,
                     td.speed, td.progress, ts->getProgress());
}

void TransferJobStatusScreen::addTransferDetails(unsigned int y, const std::string & timespent,
    const std::string & transferred, const std::string & file, const std::string & timeremaining,
    const std::string & speed, const std::string & progress, int progresspercent) {
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = table.addTextButtonNoContent(y, 1, "timespent", timespent);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = table.addTextButtonNoContent(y, 10, "transferred", transferred);
  progressmap[msotb] = progresspercent;
  msal->addElement(msotb, 6, RESIZE_CUTEND);

  msotb = table.addTextButtonNoContent(y, 10, "filename", file);
  msal->addElement(msotb, 4, 0, RESIZE_WITHLAST3, true);

  msotb = table.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = table.addTextButtonNoContent(y, 40, "speed", speed);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = table.addTextButtonNoContent(y, 50, "progress", progress);
  msal->addElement(msotb, 3, RESIZE_REMOVE);
}

std::string TransferJobStatusScreen::getRoute(std::shared_ptr<TransferJob> tj) {
  std::string route;
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD:
      route = tj->getSrc()->getSite()->getName() + " -> /\\";
      break;
    case TRANSFERJOB_UPLOAD:
      route = "/\\ -> " + tj->getDst()->getSite()->getName();
      break;
    case TRANSFERJOB_FXP:
      route = tj->getSrc()->getSite()->getName() + " -> " +
      tj->getDst()->getSite()->getName();
      break;
  }
  return route;
}
