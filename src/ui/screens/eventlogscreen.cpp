#include "eventlogscreen.h"

#include "../../eventlog.h"
#include "../../rawbuffer.h"
#include "../../globalcontext.h"
#include "../../util.h"

#include "../ui.h"

EventLogScreen::EventLogScreen(Ui * ui) {
  this->ui = ui;
}

void EventLogScreen::initialize(unsigned int row, unsigned int col) {
  expectbackendpush = true;
  this->rawbuf = global->getEventLog()->getRawBuffer();
  rawbuf->setUiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  filtermodeinput = false;
  filtermodeinputregex = false;
  init(row, col);
}

void EventLogScreen::redraw() {
  ui->erase();
  rawbuf->bookmark();
  unsigned int rows = (filtermodeinput || filtermodeinputregex) ? row - 2 : row;
  std::list<std::string> printlines;
  bool negative = false;
  if (rawbuf->isFiltered()) {
    std::string ftext = filtertext;
    negative = !ftext.empty() && ftext[0] == '!';
    if (negative) {
      ftext = filtertext.substr(1);
    }
  }
  unsigned int size = rawbuf->isFiltered() ? rawbuf->getFilteredSize() : rawbuf->getSize();
  if (readfromcopy) {
    size = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
  }
  unsigned int numlinestoprint = size < rows ? size : rows;
  for (unsigned int i = 0; i < numlinestoprint; ++i) {
    unsigned int pos = numlinestoprint - i - 1;
    const std::pair<std::string, std::string> & entry = readfromcopy ?
        (rawbuf->isFiltered() ? rawbuf->getFilteredLineCopy(pos + copyreadpos) :
        rawbuf->getLineCopy(pos + copyreadpos))
     : (rawbuf->isFiltered() ? rawbuf->getFilteredLine(pos) : rawbuf->getLine(pos));
    std::string line = entry.first + " " + entry.second;
    ui->printStr(i, 0, line);
  }
  std::string oldtext = filterfield.getData();
  filterfield = MenuSelectOptionTextField("filter", row - 1, 1, "", oldtext, col - 20, 512, false);
  update();
}

void EventLogScreen::update() {
  if (rawbuf->linesSinceBookmark()) {
    redraw();
    return;
  }
  if (filtermodeinput || filtermodeinputregex) {
    ui->showCursor();
    std::string pretag = filtermodeinput ? "[Filter(s)]: " : "[Regex filter]: ";
    ui->printStr(filterfield.getRow(), filterfield.getCol(), pretag + filterfield.getContentText());
    ui->moveCursor(filterfield.getRow(), filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

bool EventLogScreen::keyPressed(unsigned int ch) {
  if (filtermodeinput || filtermodeinputregex) {
    if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
        ch == KEY_RIGHT || ch == KEY_LEFT || ch == KEY_DC || ch == KEY_HOME ||
        ch == KEY_END)
    {
      filterfield.inputChar(ch);
      ui->update();
      return true;
    }
    else if (ch == 10) {
      ui->hideCursor();
      filtertext = filterfield.getData();
      if (filtertext.length()) {
        if (filtermodeinput) {
          rawbuf->setWildcardFilters(util::trim(util::split(filtertext)));
        }
        else {
          try {
            std::regex regexfilter = util::regexParse(filtertext);
            rawbuf->setRegexFilter(regexfilter);
          }
          catch (std::regex_error& e) {
            ui->goInfo("Invalid regular expression.");
            return true;
          }
        }
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      filtermodeinput = false;
      filtermodeinputregex = false;
      ui->redraw();
      ui->setLegend();
      return true;
    }
    else if (ch == 27) {
      if (!filterfield.getData().empty()) {
        filterfield.clear();
        ui->update();
      }
      else {
        filtermodeinput = false;
        filtermodeinputregex = false;
        ui->hideCursor();
        ui->redraw();
        ui->setLegend();
      }
      return true;
    }
    else if (ch == '\t') {
      filtermodeinput = !filtermodeinput;
      filtermodeinputregex = !filtermodeinputregex;
      ui->update();
      ui->setLegend();
      return true;
    }
  }
  switch(ch) {
    case KEY_PPAGE:
      if (!readfromcopy) {
        rawbuf->freezeCopy();
        copyreadpos = 0;
        readfromcopy = true;
      }
      else {
        unsigned int copysize = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
        copyreadpos = copyreadpos + row / 2;
        if (row >= copysize) {
          copyreadpos = 0;
        }
        else if (copyreadpos + row > copysize) {
          copyreadpos = copysize - row;
        }
      }
      ui->redraw();
      return true;
    case KEY_NPAGE:
      if (readfromcopy) {
        if (copyreadpos == 0) {
          readfromcopy = false;
        }
        else if (copyreadpos < row / 2) {
          copyreadpos = 0;
        }
        else {
          copyreadpos = copyreadpos - row / 2;
        }
      }
      ui->redraw();
      return true;
    case 'f':
      if (rawbuf->isFiltered()) {
        rawbuf->unsetFilters();
        ui->setInfo();
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      else {
        filtermodeinput = true;
        ui->setLegend();
      }
      ui->redraw();
      return true;
    case 'F':
      if (rawbuf->isFiltered()) {
        rawbuf->unsetFilters();
        ui->setInfo();
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      else {
        filtermodeinputregex = true;
        ui->setLegend();
      }
      ui->redraw();
      return true;
    case 27: // esc
      rawbuf->setUiWatching(false);
      rawbuf->unsetFilters();
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EventLogScreen::getLegendText() const {
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Tab] switch mode - [Esc] Cancel";
  }
  if (filtermodeinputregex) {
    return "[Any] Enter regex input - [Tab] switch mode - [Esc] Cancel";
  }
  return "[Pgup] Scroll up - [Pgdn] Scroll down - [ESC/Enter] Return - Toggle [f]iltering";
}

std::string EventLogScreen::getInfoLabel() const {
  return "EVENT LOG";
}

std::string EventLogScreen::getInfoText() const {
  if (rawbuf->isFiltered()) {
    return "FILTERING ON: " + filtertext;
  }
  return "";
}
