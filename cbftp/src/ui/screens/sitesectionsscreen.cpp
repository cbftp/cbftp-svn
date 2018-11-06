#include "sitesectionsscreen.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../path.h"
#include "../../site.h"
#include "../../util.h"

namespace {

bool sectionNameCompare(const std::pair<std::string, Path> a, const std::pair<std::string, Path> b) {
  return a.first.compare(b.first) < 0;
}

}

SiteSectionsScreen::SiteSectionsScreen(Ui * ui) {
  this->ui = ui;
}

SiteSectionsScreen::~SiteSectionsScreen() {

}

void SiteSectionsScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site) {
  this->site = site;
  sitecopy = std::make_shared<Site>(*site.get());
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = -1;
  hascontents = false;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void SiteSectionsScreen::redraw() {
  ui->erase();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  totallistsize = sitecopy->sectionsSize();
  table.reset();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addSectionTableHeader(y++, table);
  while (ypos && ypos >= totallistsize) {
    --ypos;
  }
  unsigned int pos = 0;
  std::vector<std::pair<std::string, Path>> sections;
  for (auto it = sitecopy->sectionsBegin(); it != sitecopy->sectionsEnd(); ++it) {
    sections.push_back(*it);
  }
  std::sort(sections.begin(), sections.end(), sectionNameCompare);
  for (auto it = sections.begin(); it != sections.end() && y < row; ++it) {
    if (pos >= currentviewspan) {
      addSectionDetails(y++, table, it->first, it->second);
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

void SiteSectionsScreen::update() {
  redraw();
}

bool SiteSectionsScreen::keyPressed(unsigned int ch) {
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
    case 10:
    case 'E':
     if (hascontents) {
       std::shared_ptr<MenuSelectOptionTextButton> elem =
           std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
       ui->goEditSiteSection(sitecopy, elem->getLabelText());
     }
     return true;
    case 'A':
      ui->goAddSiteSection(sitecopy);
      return true;
    case KEY_DC:
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        sitecopy->removeSection(elem->getLabelText());
        ui->redraw();
      }
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case 'd':
      site->clearSections();
      for (auto it = sitecopy->sectionsBegin(); it != sitecopy->sectionsEnd(); ++it) {
        site->addSection(it->first, it->second.toString());
      }
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

std::string SiteSectionsScreen::getLegendText() const {
  return "[A]dd section - [Enter/E] Details - [Esc/c/d] Return - [Up/Down] Navigate - [Del]ete section";
}

std::string SiteSectionsScreen::getInfoLabel() const {
  return "SITE SECTIONS: " + sitecopy->getName();
}

std::string SiteSectionsScreen::getInfoText() const {
  return "Total: " + std::to_string(totallistsize);
}

void SiteSectionsScreen::addSectionTableHeader(unsigned int y, MenuSelectOption & mso) {
  addSectionTableRow(y, mso, false, "NAME", "PATH");
}

void SiteSectionsScreen::addSectionDetails(unsigned int y, MenuSelectOption & mso, const std::string & section, const Path & path) {
  addSectionTableRow(y, mso, true, section, path.toString());
}

void SiteSectionsScreen::addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string & name, const std::string & path)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "name", name);
  msal->addElement(msotb, 6, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 1, "path", path);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
}
