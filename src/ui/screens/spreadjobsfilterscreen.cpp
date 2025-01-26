#include "spreadjobsfilterscreen.h"

#include <list>

#include "allracesscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptioncheckbox.h"
#include "../siteselection.h"

#include "../../commandowner.h"
#include "../../site.h"
#include "../../util.h"

SpreadJobsFilterScreen::SpreadJobsFilterScreen(Ui* ui) : UIWindow(ui, "JobsFilterScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('f', KEYACTION_FILTER, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('r', KEYACTION_RESET, "Reset");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
}

SpreadJobsFilterScreen::~SpreadJobsFilterScreen() {

}

void SpreadJobsFilterScreen::initialize(unsigned int row, unsigned int col, const SpreadJobsFilteringParameters& sjfp) {
  mso.reset();
  int y = 1;
  mso.addCheckBox(y++, 1, "jobnamefilter", "Enable job name filtering:", sjfp.usejobnamefilter);
  mso.addStringField(y++, 1, "jobname", "Job name:", sjfp.jobnamefilter, false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "sitesfilter", "Enable sites filtering:", sjfp.usesitefilter);
  mso.addStringField(y++, 1, "anysites", "Any of these sites:", util::join(sjfp.anysitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "allsites", "All of these sites:", util::join(sjfp.allsitefilters, ","), false, 60, 2048);
  y++;
  mso.addCheckBox(y++, 1, "statusfilter", "Enable status filtering:", sjfp.usestatusfilter);
  mso.addCheckBox(y++, 1, "statusinprogress", "In progress:", sjfp.showstatusinprogress);
  mso.addCheckBox(y++, 1, "statusdone", "Done:", sjfp.showstatusdone);
  mso.addCheckBox(y++, 1, "statusabort", "Aborted:", sjfp.showstatusaborted);
  mso.addCheckBox(y++, 1, "statustimeout", "Timeout:", sjfp.showstatustimeout);

  mso.enterFocusFrom(0);
  init(row, col);
}

void SpreadJobsFilterScreen::redraw() {
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

void SpreadJobsFilterScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->setText(arg);
    std::list<std::string> items = util::trim(util::split(arg, ","));
    if (!items.empty()) {
      std::string identifier = activeelement->getIdentifier();
      if (identifier == "anysites" || identifier == "allsites") {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->setValue(true);
      }
    }
    ui->redraw();
  }
}

bool SpreadJobsFilterScreen::onDeactivated(const std::shared_ptr<MenuSelectOptionElement>& msoe) {
  if (msoe->getIdentifier() == "jobname" && !std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData().empty()) {
    std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobnamefilter"))->setValue(true);
  }
  return false;
}

bool SpreadJobsFilterScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
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
      SpreadJobsFilteringParameters sjfp;
      sjfp.usejobnamefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobnamefilter"))->getData();
      sjfp.jobnamefilter = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("jobname"))->getData();
      sjfp.usesitefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->getData();
      sjfp.anysitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("anysites"))->getData(), ","));
      sjfp.allsitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("allsites"))->getData(), ","));
      sjfp.usestatusfilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfilter"))->getData();
      sjfp.showstatusinprogress = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusinprogress"))->getData();
      sjfp.showstatusdone = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusdone"))->getData();
      sjfp.showstatusaborted = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusabort"))->getData();
      sjfp.showstatustimeout = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statustimeout"))->getData();
      ui->returnSpreadJobsFilters(sjfp);
      return true;
    }
    case KEYACTION_ENTER:
    {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      std::string identifier = msoe->getIdentifier();
      if (identifier == "anysites" || identifier == "allsites") {
        std::string preselectstr = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
        std::list<std::shared_ptr<Site>> preselected;
        fillPreselectionList(preselectstr, &preselected);
        activeelement = msoe;
        std::string headerword = identifier == "anysites" ? "any" : "all";
        ui->goSelectSites("Show jobs containing " + headerword + " of these sites", preselected, std::list<std::shared_ptr<Site>>());
        return true;
      }
      bool activation = msoe->activate();
      if (!activation) {
        if ((identifier == "statusinprogress" || identifier == "statusdone" || identifier == "statusabort" || identifier == "statustimeout") &&
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
      initialize(row, col, SpreadJobsFilteringParameters());
      ui->redraw();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string SpreadJobsFilterScreen::getInfoLabel() const {
  return "SPREAD JOBS FILTERING";
}

