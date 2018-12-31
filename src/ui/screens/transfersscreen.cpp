#include "transfersscreen.h"

#include "transferstatusscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../transferstatus.h"
#include "../../transfermanager.h"
#include "../../util.h"

namespace {

bool filteringActive(const TransferFilteringParameters & tfp) {
  bool jobfilter = tfp.usejobfilter && (!tfp.spreadjobsfilter.empty() || !tfp.transferjobsfilter.empty());
  bool sitefilter = tfp.usesitefilter && (!tfp.sourcesitefilters.empty() || !tfp.targetsitefilters.empty() || !tfp.anydirectionsitefilters.empty());
  bool filenamefilter = tfp.usefilenamefilter && !tfp.filenamefilter.empty();
  bool statusfilter = tfp.usestatusfilter && (tfp.showstatusinprogress || tfp.showstatusdone || tfp.showstatusfail || tfp.showstatusdupe);
  return jobfilter || sitefilter || filenamefilter || statusfilter;
}

std::string getFilterText(const TransferFilteringParameters & tfp) {
  std::string output;
  int filters = 0;
  bool jobfilter = tfp.usejobfilter && (!tfp.spreadjobsfilter.empty() || !tfp.transferjobsfilter.empty());
  bool sitefilter = tfp.usesitefilter && (!tfp.sourcesitefilters.empty() || !tfp.targetsitefilters.empty() || !tfp.anydirectionsitefilters.empty());
  bool filenamefilter = tfp.usefilenamefilter && !tfp.filenamefilter.empty();
  bool statusfilter = tfp.usestatusfilter && (tfp.showstatusinprogress || tfp.showstatusdone || tfp.showstatusfail || tfp.showstatusdupe);
  if (jobfilter) {
    filters++;
  }
  if (sitefilter) {
    filters++;
  }
  if (filenamefilter) {
    filters++;
  }
  if (statusfilter) {
    filters++;
  }

  if (jobfilter) {
    size_t size = tfp.spreadjobsfilter.size() + tfp.transferjobsfilter.size();
    if (size > 1 || filters > 1) {
      output += "jobs";
      if (filters > 1) {
        output += ",";
      }
    }
    else if (size == 1) {
      if (!tfp.spreadjobsfilter.empty()) {
        output += "job: " + tfp.spreadjobsfilter.front();
      }
      else if (!tfp.transferjobsfilter.empty()) {
        output += "job: " + tfp.transferjobsfilter.front();
      }
    }
  }
  if (sitefilter) {
    int sitefilters = 0;
    if (!tfp.sourcesitefilters.empty()) {
      sitefilters++;
    }
    if (!tfp.targetsitefilters.empty()) {
      sitefilters++;
    }
    if (!tfp.anydirectionsitefilters.empty()) {
      sitefilters++;
    }
    if (filters > 1 || sitefilters > 1) {
      output += "sites";
      if (filters > 1) {
        output += ",";
      }
    }
    else {
      if (!tfp.sourcesitefilters.empty()) {
        std::string sitelist = util::join(tfp.sourcesitefilters, ",");
        if (sitelist.length() > 10) {
          output += "source sites";
        }
        else {
          output += "source site";
          output += (tfp.sourcesitefilters.size() > 1 ? "s: " : ": ") + sitelist;
        }
      }
      else if (!tfp.targetsitefilters.empty()) {
        std::string sitelist = util::join(tfp.targetsitefilters, ",");
        if (sitelist.length() > 10) {
          output += "target sites";
        }
        else {
          output += "target site";
          output += (tfp.targetsitefilters.size() > 1 ? "s: " : ": ") + sitelist;
        }
      }
      else if (!tfp.anydirectionsitefilters.empty()) {
        std::string sitelist = util::join(tfp.anydirectionsitefilters, ",");
        if (sitelist.length() > 10) {
          output += "sites";
        }
        else {
          output += "site";
          output += (tfp.anydirectionsitefilters.size() > 1 ? "s: " : ": ") + sitelist;
        }
      }
    }
  }
  if (filenamefilter) {
    if (filters > 1) {
      output += "filename,";
    }
    else {
      output += "filename: " + tfp.filenamefilter;
    }
  }
  if (statusfilter) {
    if (filters > 1) {
      output += "status,";
    }
    else {
      output += "status: ";
      std::string statustext;
      if (tfp.showstatusinprogress) {
        statustext += "inprogress,";
      }
      if (tfp.showstatusdone) {
        statustext += "done,";
      }
      if (tfp.showstatusfail) {
        statustext += "fail,";
      }
      if (tfp.showstatusdupe) {
        statustext += "dupe,";
      }
      if (statustext.length()) {
        output += statustext.substr(0, statustext.length() - 1);
      }
    }
  }
  if (filters > 1) {
    output = output.substr(0, output.length() - 1);
  }
  return output;
}

}

TransfersScreen::TransfersScreen(Ui * ui) {
  this->ui = ui;
  tm = global->getTransferManager();
  nextid = 0;
}

TransfersScreen::~TransfersScreen() {

}

void TransfersScreen::initialize(unsigned int row, unsigned int col) {
  initialize(row, col, TransferFilteringParameters());
}

void TransfersScreen::initializeFilterSite(unsigned int row, unsigned int col, const std::string & site) {
  TransferFilteringParameters tfp;
  tfp.usesitefilter = true;
  tfp.anydirectionsitefilters.push_back(site);
  initialize(row, col, tfp);
}

void TransfersScreen::initializeFilterSpreadJob(unsigned int row, unsigned int col, const std::string & job) {
  TransferFilteringParameters tfp;
  tfp.usejobfilter = true;
  tfp.spreadjobsfilter.push_back(job);
  initialize(row, col, tfp);
}

void TransfersScreen::initializeFilterTransferJob(unsigned int row, unsigned int col, const std::string & job) {
  TransferFilteringParameters tfp;
  tfp.usejobfilter = true;
  tfp.transferjobsfilter.push_back(job);
  initialize(row, col, tfp);
}

void TransfersScreen::initialize(unsigned int row, unsigned int col, const TransferFilteringParameters & tfp) {
  filtering = filteringActive(tfp);
  this->tfp = tfp;
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  finishedfilteredtransfers.clear();
  numfinishedfiltered = 0;
  addFilterFinishedTransfers();
  ypos = 0;
  temphighlightline = -1;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

bool TransfersScreen::showsWhileFiltered(const std::shared_ptr<TransferStatus> & ts) const {
  if (tfp.usejobfilter) {
    bool match = false;
    if (!tfp.spreadjobsfilter.empty()) {
      for (std::list<std::string>::const_iterator it = tfp.spreadjobsfilter.begin(); it != tfp.spreadjobsfilter.end(); it++) {
        if (ts->getTargetPath().toString().find(*it) != std::string::npos) {
          match = true;
          break;
        }
      }
    }
    if (!match && !tfp.transferjobsfilter.empty()) {
      for (std::list<std::string>::const_iterator it = tfp.transferjobsfilter.begin(); it != tfp.transferjobsfilter.end(); it++) {
        if (ts->getTargetPath().toString().find(*it) != std::string::npos) {
          match = true;
          break;
        }
      }
    }
    if (!match) {
      return false;
    }
  }
  if (tfp.usesitefilter) {
    if (!tfp.sourcesitefilters.empty()) {
      bool match = false;
      for (std::list<std::string>::const_iterator it = tfp.sourcesitefilters.begin(); it != tfp.sourcesitefilters.end(); it++) {
        if (ts->getSource() == *it) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
    if (!tfp.targetsitefilters.empty()) {
      bool match = false;
      for (std::list<std::string>::const_iterator it = tfp.targetsitefilters.begin(); it != tfp.targetsitefilters.end(); it++) {
        if (ts->getTarget() == *it) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
    if (!tfp.anydirectionsitefilters.empty() && (tfp.sourcesitefilters.empty() || tfp.targetsitefilters.empty())) {
      bool match = false;
      for (std::list<std::string>::const_iterator it = tfp.anydirectionsitefilters.begin(); it != tfp.anydirectionsitefilters.end(); it++) {
        if (ts->getSource() == *it || ts->getTarget() == *it) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
  }
  if (tfp.usefilenamefilter && !tfp.filenamefilter.empty()) {
    if (!util::wildcmp(tfp.filenamefilter.c_str(), ts->getFile().c_str())) {
      return false;
    }
  }
  if (tfp.usestatusfilter && (tfp.showstatusinprogress || tfp.showstatusdone || tfp.showstatusfail || tfp.showstatusdupe)) {
    switch (ts->getState()) {
      case TRANSFERSTATUS_STATE_IN_PROGRESS:
        if (!tfp.showstatusinprogress) {
          return false;
        }
        break;
      case TRANSFERSTATUS_STATE_SUCCESSFUL:
        if (!tfp.showstatusdone) {
          return false;
        }
        break;
      case TRANSFERSTATUS_STATE_FAILED:
      case TRANSFERSTATUS_STATE_ABORTED:
        if (!tfp.showstatusfail) {
          return false;
        }
        break;
      case TRANSFERSTATUS_STATE_DUPE:
        if (!tfp.showstatusdupe) {
          return false;
        }
    }
  }
  return true;
}

void TransfersScreen::addFilterFinishedTransfers() {
  if (!filtering) {
    return;
  }
  int i = 0;
  size_t finishedsize = tm->totalFinishedTransfers();
  int count = finishedsize - numfinishedfiltered;
  std::list<std::shared_ptr<TransferStatus> > newfiltered;
  for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd() && i < count; it++, i++) {
    if (showsWhileFiltered(*it)) {
      newfiltered.push_back(*it);
    }
  }
  finishedfilteredtransfers.splice(finishedfilteredtransfers.begin(), newfiltered);
  if (finishedfilteredtransfers.size() > MAX_TRANSFER_HISTORY) {
    finishedfilteredtransfers.resize(MAX_TRANSFER_HISTORY);
  }
  numfinishedfiltered = finishedsize;
}

unsigned int TransfersScreen::totalListSize() const {
  int ongoingfiltercount = 0;
  if (filtering) {
    for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd(); it++) {
      if (showsWhileFiltered(*it)) {
        ongoingfiltercount++;
      }
    }
  }
  return filtering ? ongoingfiltercount + finishedfilteredtransfers.size() : tm->ongoingTransfersSize() + tm->finishedTransfersSize();
}

void TransfersScreen::redraw() {
  ui->erase();
  addFilterFinishedTransfers();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  unsigned int totallistsize = totalListSize();
  table.reset();
  statusmap.clear();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addTransferTableHeader(y++, table);

  unsigned int pos = 0;
  for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd() && y < row; it++) {
    if (filtering && !showsWhileFiltered(*it)) {
      continue;
    }
    if (pos >= currentviewspan) {
      int id = nextid++;
      addTransferDetails(y++, table, *it, id);
      statusmap[id] = *it;
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  if (filtering) {
    for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = finishedfilteredtransfers.begin(); it != finishedfilteredtransfers.end() && y < row; it++) {
      if (pos >= currentviewspan) {
        int id = nextid++;
        addTransferDetails(y++, table, *it, id);
        statusmap[id] = *it;
        if (pos == ypos) {
          table.enterFocusFrom(2);
        }
      }
      ++pos;
    }
  }
  else {
    for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd() && y < row; it++) {
      if (pos >= currentviewspan) {
        int id = nextid++;
        addTransferDetails(y++, table, *it, id);
        statusmap[id] = *it;
        if (pos == ypos) {
          table.enterFocusFrom(2);
        }
      }
      ++pos;
    }
  }
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  if (temphighlightline != -1) {
    std::shared_ptr<MenuSelectAdjustableLine> highlightline = table.getAdjustableLineOnRow(temphighlightline);
    if (!!highlightline) {
      std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
      for (unsigned int i = minmaxcol.first; i <= minmaxcol.second; i++) {
        ui->printChar(temphighlightline, i, ' ', true);
      }
    }
  }
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    highlight = false;
    if (hascontents && (table.getSelectionPointer() == i  || (int)re->getRow() == temphighlightline)) {
      highlight = true;
    }
    if (re->isVisible()) {
      if (re->getIdentifier() == "transferred") {
        int progresspercent = 0;
        std::map<int, std::shared_ptr<TransferStatus> >::iterator it = statusmap.find(re->getId());
        if (it != statusmap.end()) {
          progresspercent = it->second->getProgress();
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
  printSlider(ui, row, 1, col - 1, totallistsize, currentviewspan);
}

void TransfersScreen::update() {
  redraw();
}

bool TransfersScreen::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return true;
    }
  }
  switch (ch) {
    case KEY_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (hascontents && ypos < totalListSize() - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEY_NPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      unsigned int totallistsize = totalListSize();
      for (unsigned int i = 0; i < pagerows && ypos < totallistsize - 1; i++) {
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEY_PPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos > 0; i++) {
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEY_HOME:
      ypos = 0;
      ui->update();
      return true;
    case KEY_END:
      ypos = totalListSize() - 1;
      ui->update();
      return true;
    case 10:
     if (hascontents) {
       std::shared_ptr<MenuSelectOptionTextButton> elem =
           std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
       std::map<int, std::shared_ptr<TransferStatus> >::iterator it = statusmap.find(elem->getId());
       if (it != statusmap.end()) {
         ui->goTransferStatus(it->second);
       }
     }
     return true;
    case 'f':
      if (!filtering) {
        ui->goTransfersFiltering(tfp);
      }
      else {
        filtering = false;
        ui->setInfo();
        ui->redraw();
      }
      return true;
    case 'B':
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        std::map<int, std::shared_ptr<TransferStatus> >::iterator it = statusmap.find(elem->getId());
        if (it != statusmap.end()) {
          TransferStatusScreen::abortTransfer(it->second);
        }
      }
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case '-':
      if (!hascontents) {
        break;
      }
      temphighlightline = table.getElement(table.getSelectionPointer())->getRow();
      ui->redraw();
      return true;
  }
  return false;
}

std::string TransfersScreen::getLegendText() const {
  return "[Esc/c] Return - [Up/Down] Navigate - [Enter] Details - toggle [f]iltering - A[B]ort transfer";
}

std::string TransfersScreen::getInfoLabel() const {
  return "TRANSFERS";
}

std::string TransfersScreen::getInfoText() const {
  std::string output;
  if (filtering) {
    output += "FILTERING: " + getFilterText(tfp);
  }
  return output;
}

void TransfersScreen::addTransferTableHeader(unsigned int y, MenuSelectOption & mso) {
  addTransferTableRow(y, mso, false, "STARTED", "USE", "ROUTE", "PATH", "TRANSFERRED", "FILENAME", "LEFT", "SPEED", "DONE", -1);
}

void TransfersScreen::addTransferDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<TransferStatus> ts, int id) {
  TransferDetails td = formatTransferDetails(ts);
  addTransferTableRow(y, mso, true, ts->getTimestamp(), td.timespent, td.route, td.path, td.transferred,
      ts->getFile(), td.timeremaining, td.speed, td.progress, id);
}

TransferDetails TransfersScreen::formatTransferDetails(std::shared_ptr<TransferStatus> & ts) {
  TransferDetails td;
  td.route = ts->getSource() + " -> " + ts->getTarget();
  td.path = ts->getSourcePath().toString() + " -> " + ts->getTargetPath().toString();
  td.speed = util::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  td.timespent = util::simpleTimeFormat(ts->getTimeSpent());
  td.timeremaining = "-";
  td.transferred = util::parseSize(ts->targetSize());
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS: {
      int progresspercent = ts->getProgress();
      td.progress = std::to_string(progresspercent) + "%";
      int timeremainingnum = ts->getTimeRemaining();
      if (timeremainingnum < 0) {
        td.timeremaining = "?";
      }
      else {
        td.timeremaining = util::simpleTimeFormat(timeremainingnum);
      }
      break;
    }
    case TRANSFERSTATUS_STATE_FAILED:
      td.speed = "-";
      td.transferred = "-";
      td.progress = "fail";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      td.progress = "done";
      break;
    case TRANSFERSTATUS_STATE_DUPE:
      td.speed = "-";
      td.transferred = "-";
      td.progress = "dupe";
      break;
    case TRANSFERSTATUS_STATE_ABORTED:
      td.progress = "abor";
      break;
  }
  td.transferred += " / " + util::parseSize(ts->sourceSize());
  return td;
}

void TransfersScreen::addTransferTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string & timestamp, const std::string & timespent, const std::string & route,
    const std::string & path, const std::string & transferred, const std::string & filename,
    const std::string & timeremaining, const std::string & speed, const std::string & progress, int id)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 30, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 10, "path", path);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);

  msotb = mso.addTextButtonNoContent(y, 10, "transferred", transferred);
  msotb->setSelectable(false);
  msotb->setId(id);
  msal->addElement(msotb, 5, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 10, "filename", filename);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 4, 1, RESIZE_WITHLAST3, true);

  msotb = mso.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 40, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 50, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
}
