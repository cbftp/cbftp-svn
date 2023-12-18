#include "transferjobsfilterscreen.h"

#include <list>

#include "alltransferjobsscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptioncheckbox.h"
#include "../siteselection.h"

#include "../../commandowner.h"
#include "../../site.h"
#include "../../util.h"

TransferJobsFilterScreen::TransferJobsFilterScreen(Ui* ui) : UIWindow(ui, "JobsFilterScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('f', KEYACTION_FILTER, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('r', KEYACTION_RESET, "Reset");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
}

TransferJobsFilterScreen::~TransferJobsFilterScreen() {

}

void TransferJobsFilterScreen::initialize(unsigned int row, unsigned int col, const TransferJobsFilteringParameters& tjfp) {
  mso.reset();
  active = false;
  int y = 1;
  mso.addCheckBox(y++, 1, "jobnamefilter", "Enable job name filtering:", tjfp.usejobnamefilter);
  mso.addStringField(y++, 1, "jobname", "Job name:", tjfp.jobnamefilter, false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "sitesfilter", "Enable sites filtering:", tjfp.usesitefilter);
  mso.addStringField(y++, 1, "source", "Source:", util::join(tjfp.sourcesitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "destination", "Destination:", util::join(tjfp.targetsitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "anydirection", "Any direction:", util::join(tjfp.anydirectionsitefilters, ","), false, 60, 2048);
  y++;
  mso.addCheckBox(y++, 1, "statusfilter", "Enable job status filtering:", tjfp.usestatusfilter);
  mso.addCheckBox(y++, 1, "statusqueued", "Queued:", tjfp.showstatusqueued);
  mso.addCheckBox(y++, 1, "statusinprogress", "In progress:", tjfp.showstatusinprogress);
  mso.addCheckBox(y++, 1, "statusdone", "Done:", tjfp.showstatusdone);
  mso.addCheckBox(y++, 1, "statusabort", "Aborted:", tjfp.showstatusaborted);

  mso.enterFocusFrom(0);
  init(row, col);
}

void TransferJobsFilterScreen::redraw() {
  vv->clear();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
      if (active && msoe->cursorPosition() >= 0) {
        ui->showCursor();
        vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
      }
      else {
        ui->hideCursor();
      }
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void TransferJobsFilterScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->setText(arg);
    std::list<std::string> items = util::trim(util::split(arg, ","));
    if (!items.empty()) {
      std::string identifier = activeelement->getIdentifier();
      if (identifier == "source" || identifier == "destination" || identifier == "anydirection") {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->setValue(true);
      }
    }
    ui->redraw();
  }
}

bool TransferJobsFilterScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      if (activeelement->getIdentifier() == "jobname" && !std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->getData().empty()) {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobnamefilter"))->setValue(true);
      }
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  switch (action) {
    case KEYACTION_UP: {
      bool moved = mso.goUp();
      ui->update();
      return moved;
    }
    case KEYACTION_DOWN: {
      bool moved = mso.goDown();
      ui->update();
      return moved;
    }
    case KEYACTION_DONE:
    case KEYACTION_FILTER:
    {
      TransferJobsFilteringParameters tjfp;
      tjfp.usejobnamefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobnamefilter"))->getData();
      tjfp.jobnamefilter = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("jobname"))->getData();
      tjfp.usesitefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->getData();
      tjfp.sourcesitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("source"))->getData(), ","));
      tjfp.targetsitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("destination"))->getData(), ","));
      tjfp.anydirectionsitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("anydirection"))->getData(), ","));
      tjfp.usestatusfilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfilter"))->getData();
      tjfp.showstatusqueued = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusqueued"))->getData();
      tjfp.showstatusinprogress = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusinprogress"))->getData();
      tjfp.showstatusdone = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusdone"))->getData();
      tjfp.showstatusaborted = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusabort"))->getData();
      ui->returnTransferJobsFilters(tjfp);
      return true;
    }
    case KEYACTION_ENTER:
    {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "source" || msoe->getIdentifier() == "destination" || msoe->getIdentifier() == "anydirection") {
        std::string preselectstr = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
        std::list<std::shared_ptr<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        activeelement = msoe;
        std::string headerword = msoe->getIdentifier() + " ";
        if (headerword == "anydirection") {
          headerword = "";
        }
        ui->goSelectSites("Show jobs containing these " + headerword + "sites", preselected, std::list<std::shared_ptr<Site>>());
        return true;
      }
      bool activation = msoe->activate();
      if (!activation) {
        std::string identifier = msoe->getIdentifier();
        if ((identifier == "statusqueued" || identifier == "statusinprogress" || identifier == "statusdone" || identifier == "statusabort") &&
            std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData())
        {
          std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfilter"))->setValue(true);
        }
        ui->update();
        return true;
      }
      active = true;
      activeelement = msoe;
      ui->setLegend();
      ui->update();
      return true;
    }
    case KEYACTION_RESET:
      initialize(row, col, TransferJobsFilteringParameters());
      ui->redraw();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransferJobsFilterScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string TransferJobsFilterScreen::getInfoLabel() const {
  return "TRANSFER JOBS FILTERING";
}
