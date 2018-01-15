#include "selectjobsscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptioncheckbox.h"
#include "../misc.h"

#include "../../commandowner.h"
#include "../../race.h"
#include "../../transferjob.h"
#include "../../engine.h"
#include "../../globalcontext.h"
#include "../../util.h"

SelectJobsScreen::SelectJobsScreen(Ui * ui) {
  this->ui = ui;
}

void SelectJobsScreen::initialize(unsigned int row, unsigned int col, JobType type) {
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  numselected = 0;
  this->type = type;

  table.reset();
  unsigned int y = 0;
  addJobTableHeader(y++, table, "JOB NAME");
  if (type == JOBTYPE_SPREADJOB) {
    totallistsize = global->getEngine()->allRaces();
    for (std::list<Pointer<Race> >::const_iterator it = --global->getEngine()->getRacesEnd();
        it != --global->getEngine()->getRacesBegin(); --it, ++y)
    {
      addJobDetails(y, table, *it);
    }
  }
  else {
    totallistsize = global->getEngine()->allTransferJobs();
    for (std::list<Pointer<TransferJob> >::const_iterator it = --global->getEngine()->getTransferJobsEnd();
        it != --global->getEngine()->getTransferJobsBegin(); --it, ++y)
    {
      addJobDetails(y, table, *it);
    }
  }
  table.checkPointer();
  init(row, col);
}

void SelectJobsScreen::redraw() {
  ui->erase();
  unsigned int listspan = row - 1;
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  bool highlight;
  int y = 0;
  for (unsigned int i = 0; i < table.linesSize(); i++) {
    if (i != 0 && (i < currentviewspan + 1 || i > currentviewspan + listspan)) {
      continue;
    }
    Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(i);
    for (unsigned int j = 0; j < msal->size(); ++j) {
      Pointer<ResizableElement> re = msal->getElement(j);
      highlight = false;
      if (j == 1 && ypos + 1 == currentviewspan + y && hascontents) {
        highlight = true;
      }
      if (re->isVisible()) {
        ui->printStr(y, re->getCol(), re->getLabelText(), highlight);
      }
    }
    y++;
  }
  printSlider(ui, row, 1, col - 1, totallistsize, currentviewspan);
}

void SelectJobsScreen::update() {
  redraw();
}

bool SelectJobsScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case KEY_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (hascontents && ypos < totallistsize - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEY_NPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
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
      ypos = totallistsize - 1;
      ui->update();
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case 10:
    case 32:
      if (hascontents) {
        Pointer<MenuSelectOptionTextButton> msotb =
            table.getElement(table.getSelectionPointer());
        Pointer<MenuSelectAdjustableLine> line = table.getAdjustableLine(msotb);
        Pointer<MenuSelectOptionCheckBox> checkbox = line->getElement(0);
        checkbox->activate();
        if (checkbox->getData()) {
          numselected++;
          checkbox->setLabel("[X]");
        }
        else {
          numselected--;
          checkbox->setLabel("[ ]");
        }
      }
      ui->setInfo();
      ui->update();
      return true;
    case 'd': {
      std::list<std::string> items;
      for (unsigned int i = 0; i < table.size(); i++) {
        Pointer<ResizableElement> re = table.getElement(i);
        if (re->getIdentifier() == "selected" && re.get<MenuSelectOptionCheckBox>()->getData()) {
          Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(re);
          Pointer<MenuSelectOptionTextButton> name = msal->getElement(1);
          items.push_back(util::int2Str(name->getId()));
        }
      }
      ui->returnSelectItems(util::join(items, ","));
      return true;
    }
  }
  return false;
}

std::string SelectJobsScreen::getLegendText() const {
  return "[Esc/c] Return - [Enter/space] Select - [Up/Down/Pgup/Pgdn/Home/End] Navigate - [d]one";
}

std::string SelectJobsScreen::getInfoLabel() const {
  return std::string("SELECT ") + (type == JOBTYPE_SPREADJOB ? "SPREAD" : "TRANSFER") + " JOBS";
}

std::string SelectJobsScreen::getInfoText() const {
  return "Selected: " + util::int2Str(numselected);
}

void SelectJobsScreen::addJobTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & release) {
  addTableRow(y, mso, -1, false, "JOB NAME", "USE");
}

void SelectJobsScreen::addTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & name, const std::string & checkboxtext)
{
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<ResizableElement> re;

  re = mso.addCheckBox(y,  1, "selected",  checkboxtext, false);
  re->setSelectable(false);
  msal->addElement(re, 2, 1, RESIZE_REMOVE, false);

  re = mso.addTextButton(y, 1, "name", name);
  re->setSelectable(selectable);
  re->setId(id);
  msal->addElement(re, 1, 1, RESIZE_CUTEND, true);
}

void SelectJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, Pointer<Race> job) {
  addTableRow(y, mso, job->getId(), true, job->getName(), "[ ]");
}

void SelectJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, Pointer<TransferJob> job) {
  addTableRow(y, mso, job->getId(), true, job->getName(), "[ ]");
}
