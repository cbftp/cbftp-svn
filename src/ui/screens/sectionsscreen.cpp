#include "sectionsscreen.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../section.h"
#include "../../sectionmanager.h"
#include "../../skiplist.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../util.h"

namespace {

bool sectionNameCompare(const Section * a, const Section * b) {
  return a->getName().compare(b->getName()) < 0;
}

}

SectionsScreen::SectionsScreen(Ui * ui) {
  this->ui = ui;
}

SectionsScreen::~SectionsScreen() {

}

void SectionsScreen::initialize(unsigned int row, unsigned int col, bool selectsection, const std::list<std::string>& preselected) {
  mode = selectsection ? Mode::SELECT : Mode::EDIT;
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = -1;
  hascontents = false;
  table.reset();
  table.enterFocusFrom(0);
  selected.clear();
  for (const std::string& section : preselected) {
    if (global->getSectionManager()->getSection(section)) {
      selected.insert(section);
    }
  }
  togglestate = false;
  init(row, col);
}

void SectionsScreen::redraw() {
  ui->erase();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  totallistsize = global->getSectionManager()->size();
  table.reset();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addSectionTableHeader(y++, table);
  while (ypos && ypos >= totallistsize) {
    --ypos;
  }
  unsigned int pos = 0;
  std::vector<const Section *> sections;
  for (auto it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
    sections.push_back(&it->second);
  }
  std::sort(sections.begin(), sections.end(), sectionNameCompare);
  for (auto it = sections.begin(); it != sections.end() && y < row; ++it) {
    if (pos >= currentviewspan) {
      addSectionDetails(y++, table, **it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
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
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  printSlider(ui, row, 1, col - 1, totallistsize, currentviewspan);
}

void SectionsScreen::update() {
  redraw();
}

bool SectionsScreen::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return true;
    }
  }
  if (hascontents && mode == Mode::SELECT && (ch == 10 || ch == 'd')) {
    std::string selectedstr;
    if (selected.empty()) {
      std::shared_ptr<MenuSelectOptionTextButton> elem =
          std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
      selectedstr = elem->getLabelText();
    }
    else {
      selectedstr = util::join(selected, ",");
    }
    ui->returnSelectItems(selectedstr);
    return true;
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
    case ' ':
      if (hascontents && mode == Mode::SELECT) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        std::string section = elem->getLabelText();
        if (selected.find(section) == selected.end()) {
          selected.insert(section);
        }
        else {
          selected.erase(section);
        }
        ui->redraw();
        ui->setLegend();
        return true;
      }
      break;
    case KEY_END:
      ypos = totallistsize - 1;
      ui->update();
      return true;
    case 10:
    case 'E':
     if (hascontents && mode == Mode::EDIT) {
       std::shared_ptr<MenuSelectOptionTextButton> elem =
           std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
       ui->goEditSection(elem->getLabelText());
     }
     return true;
    case 'A':
      if (mode == Mode::EDIT) {
        ui->goAddSection();
      }
      return true;
    case KEY_DC:
      if (hascontents && mode == Mode::EDIT) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        ui->goConfirmation("Are you sure that you wish to remove this section: " + elem->getLabelText());
      }
      return true;
    case 'c':
    case 'd':
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
    case 't':
      if (selected.empty() || !togglestate) {
        selected.clear();
        for (auto it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
          selected.insert(it->first);
        }
        togglestate = true;
      }
      else if (selected.size() == global->getSectionManager()->size() || togglestate) {
        selected.clear();
        togglestate = false;
      }
      ui->redraw();
      ui->setLegend();
      return true;
  }
  return false;
}

void SectionsScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    std::shared_ptr<MenuSelectOptionTextButton> elem =
        std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
    global->getSectionManager()->removeSection(elem->getLabelText());
  }
  ui->redraw();
}

std::string SectionsScreen::getLegendText() const {
  if (mode == Mode::SELECT) {
    std::string enterdesc = selected.empty() ? "Select and return" : "Return selected items";
    return "[Up/Down] Navigate - [Enter/d] " + enterdesc + " - [Esc/c] Cancel - [Space] Select - [t]oggle all";
  }
  return "[A]dd section - [Enter/E] Details - [Esc/c/d] Return - [Up/Down] Navigate - [Del]ete section";
}

std::string SectionsScreen::getInfoLabel() const {
  if (mode == Mode::SELECT) {
    return "SELECT SECTIONS";
  }
  return "SECTIONS";
}

std::string SectionsScreen::getInfoText() const {
  return "Total: " + std::to_string(totallistsize);
}

void SectionsScreen::addSectionTableHeader(unsigned int y, MenuSelectOption & mso) {
  addSectionTableRow(y, mso, false, "SEL", "NAME", "SKIPLIST", "HOTKEY", "#JOBS", "#SITES", "SITES");
}

void SectionsScreen::addSectionDetails(unsigned int y, MenuSelectOption & mso, const Section & section) {
  std::string skiplist = section.getSkipList().size() ? "[X]" : "[ ]";
  std::string selectedstr = (selected.find(section.getName()) != selected.end()) ? "[X]" : "[ ]";
  std::list<std::string> sites;
  for (auto it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
    if ((*it)->hasSection(section.getName())) {
      sites.push_back((*it)->getName());
    }
  }
  int hotkey = section.getHotKey();
  std::string hotkeystr = hotkey != -1 ? std::to_string(hotkey) : "None";
  addSectionTableRow(y, mso, true, selectedstr, section.getName(), skiplist, hotkeystr, std::to_string(section.getNumJobs()),
                     std::to_string(sites.size()), util::join(sites, ","));
}

void SectionsScreen::addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string& selected, const std::string & name, const std::string & skiplist, const std::string & hotkey, const std::string & numjobs,
    const std::string & numsites, const std::string & sites)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  if (mode == Mode::SELECT) {
    msotb = mso.addTextButtonNoContent(y, 1, "selected", selected);
    msal->addElement(msotb, 6, RESIZE_CUTEND);
  }

  msotb = mso.addTextButtonNoContent(y, 1, "name", name);
  msal->addElement(msotb, 7, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 1, "skiplist", skiplist);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "hotkey", hotkey);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "numjobs", numjobs);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "numsites", numsites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_WITHDOTS);
}
