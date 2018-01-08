#include "transfersfilterscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptioncheckbox.h"

#include "../../commandowner.h"
#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../engine.h"
#include "../../race.h"
#include "../../transferjob.h"
#include "../../util.h"

namespace {

void fillPreselectionList(const std::string & preselectstr, std::list<Pointer<Site> > * list) {
  std::list<std::string> preselectlist = util::split(preselectstr, ",");
  for (std::list<std::string>::const_iterator it = preselectlist.begin(); it != preselectlist.end(); it++) {
    Pointer<Site> site = global->getSiteManager()->getSite(*it);
    list->push_back(site);
  }
}

}
TransfersFilterScreen::TransfersFilterScreen(Ui * ui) {
  this->ui = ui;
}

TransfersFilterScreen::~TransfersFilterScreen() {

}

void TransfersFilterScreen::initialize(unsigned int row, unsigned int col, const TransferFilteringParameters & tfp) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel - [r]eset";
  currentlegendtext = defaultlegendtext;
  mso.reset();
  selectedspreadjobs = tfp.spreadjobsfilter;
  selectedtransferjobs = tfp.transferjobsfilter;
  active = false;
  int y = 1;
  mso.addCheckBox(y++, 1, "jobfilter", "Enable job filtering:", tfp.usejobfilter);
  mso.addStringField(y++, 1, "spreadjobs", "Spread jobs:", getSpreadJobsText(), false, 60, 512);
  mso.addStringField(y++, 1, "transferjobs", "Transfer jobs:", getTransferJobsText(), false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "sitesfilter", "Enable sites filtering:", tfp.usesitefilter);
  mso.addStringField(y++, 1, "source", "Source:", util::join(tfp.sourcesitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "destination", "Destination:", util::join(tfp.targetsitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "anydirection", "Any direction:", util::join(tfp.anydirectionsitefilters, ","), false, 60, 2048);
  y++;
  mso.addCheckBox(y++, 1, "filenamefilter", "Enable file name filtering:", tfp.usefilenamefilter);
  mso.addStringField(y++, 1, "filename", "File name:", tfp.filenamefilter, false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "statusfilter", "Enable transfer status filtering:", tfp.usestatusfilter);
  mso.addCheckBox(y++, 1, "statusinprogress", "In progress:", tfp.showstatusinprogress);
  mso.addCheckBox(y++, 1, "statusdone", "Done:", tfp.showstatusdone);
  mso.addCheckBox(y++, 1, "statusfail", "Failed:", tfp.showstatusfail);
  mso.addCheckBox(y++, 1, "statusdupe", "Dupe:", tfp.showstatusdupe);
  mso.enterFocusFrom(0);
  init(row, col);
}

void TransfersFilterScreen::redraw() {
  ui->erase();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
      if (active && msoe->cursorPosition() >= 0) {
        ui->showCursor();
        ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
      }
      else {
        ui->hideCursor();
      }
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void TransfersFilterScreen::update() {
  redraw();
}

void TransfersFilterScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::list<std::string> items = util::split(arg, ",");
    if (activeelement->getIdentifier() == "spreadjobs") {
      selectedspreadjobs.clear();
      for (std::list<std::string>::const_iterator it = items.begin(); it != items.end(); it++) {
        selectedspreadjobs.push_back(global->getEngine()->getRace(util::str2Int(*it))->getName());
      }
      mso.getElement("spreadjobs").get<MenuSelectOptionTextField>()->setText(getSpreadJobsText());

    }
    else if (activeelement->getIdentifier() == "transferjobs") {
      selectedtransferjobs.clear();
      for (std::list<std::string>::const_iterator it = items.begin(); it != items.end(); it++) {
        selectedtransferjobs.push_back(global->getEngine()->getTransferJob(util::str2Int(*it))->getName());
      }
      mso.getElement("transferjobs").get<MenuSelectOptionTextField>()->setText(getTransferJobsText());
    }
    else {
      activeelement.get<MenuSelectOptionTextField>()->setText(arg);
    }
    redraw();
  }
}

bool TransfersFilterScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  switch (ch) {
    case KEY_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEY_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case KEY_LEFT:
      mso.goLeft();
      ui->update();
      return true;
    case KEY_RIGHT:
      mso.goRight();
      ui->update();
      return true;
    case 'd':
    case 'f': {
      TransferFilteringParameters tfp;
      tfp.usejobfilter = mso.getElement("jobfilter").get<MenuSelectOptionCheckBox>()->getData();
      tfp.spreadjobsfilter = selectedspreadjobs;
      tfp.transferjobsfilter = selectedtransferjobs;
      tfp.usesitefilter = mso.getElement("sitesfilter").get<MenuSelectOptionCheckBox>()->getData();
      tfp.sourcesitefilters = util::split(mso.getElement("source").get<MenuSelectOptionTextField>()->getData(), ",");
      tfp.targetsitefilters = util::split(mso.getElement("destination").get<MenuSelectOptionTextField>()->getData(), ",");
      tfp.anydirectionsitefilters = util::split(mso.getElement("anydirection").get<MenuSelectOptionTextField>()->getData(), ",");
      tfp.usefilenamefilter = mso.getElement("filenamefilter").get<MenuSelectOptionCheckBox>()->getData();
      tfp.filenamefilter = mso.getElement("filename").get<MenuSelectOptionTextField>()->getData();
      tfp.usestatusfilter = mso.getElement("statusfilter").get<MenuSelectOptionCheckBox>()->getData();
      tfp.showstatusinprogress = mso.getElement("statusinprogress").get<MenuSelectOptionCheckBox>()->getData();
      tfp.showstatusdone = mso.getElement("statusdone").get<MenuSelectOptionCheckBox>()->getData();
      tfp.showstatusfail= mso.getElement("statusfail").get<MenuSelectOptionCheckBox>()->getData();
      tfp.showstatusdupe = mso.getElement("statusdupe").get<MenuSelectOptionCheckBox>()->getData();
      ui->returnTransferFilters(tfp);
      return true;
    }
    case 10: {
      Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "spreadjobs") {
        activeelement = msoe;
        ui->goSelectSpreadJobs();
        return true;
      }
      if (msoe->getIdentifier() == "transferjobs") {
        activeelement = msoe;
        ui->goSelectTransferJobs();
        return true;
      }
      if (msoe->getIdentifier() == "source" || msoe->getIdentifier() == "destination" || msoe->getIdentifier() == "anydirection") {
        std::string preselectstr = msoe.get<MenuSelectOptionTextField>()->getData();
        std::list<Pointer<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        activeelement = msoe;
        std::string headerword = msoe->getIdentifier() + " ";
        if (headerword == "anydirection") {
          headerword = "";
        }
        ui->goSelectSites("Show these " + headerword + "sites in transfers", preselected, std::list<Pointer<Site> >());
        return true;
      }
      bool activation = msoe->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = msoe;
      currentlegendtext = activeelement->getLegendText();
      ui->setLegend();
      ui->update();
      return true;
    }
    case 'r':
      initialize(row, col, TransferFilteringParameters());
      ui->redraw();
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransfersFilterScreen::getLegendText() const {
  return currentlegendtext;
}

std::string TransfersFilterScreen::getInfoLabel() const {
  return "TRANSFERS FILTERING";
}

std::string TransfersFilterScreen::getSpreadJobsText() const {
  size_t size = selectedspreadjobs.size();
  std::string text;
  if (size == 1) {
    text = selectedspreadjobs.front();
  }
  else if (size > 1) {
    text = util::int2Str((int)size) + " spread jobs selected...";
  }
  return text;
}

std::string TransfersFilterScreen::getTransferJobsText() const {
  size_t size = selectedtransferjobs.size();
  std::string text;
  if (size == 1) {
    text = selectedtransferjobs.front();
  }
  else if (size > 1) {
    text = util::int2Str((int)size) + " transfer jobs selected...";
  }
  return text;
}
