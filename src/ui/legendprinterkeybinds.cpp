#include "legendprinterkeybinds.h"

#include "ui.h"

LegendPrinterKeybinds::LegendPrinterKeybinds(Ui * ui) : ui(ui), offset(0), staticcount(0) {

}

bool LegendPrinterKeybinds::print() {
  unsigned int printpos = 4;
  if (text.length() > 0) {
    if (ui->legendMode() == LEGEND_SCROLLING) {
      std::string scrollingtext = text + "  ::  ";
      unsigned int textlen = scrollingtext.length();
      unsigned int internalpos = printpos - offset++;
      if (offset >= textlen) offset = 0;
      while (printpos < col - 4) {
        while (printpos - internalpos < textlen && printpos < col - 4) {
          ui->printChar(ui->getLegendWindow(), 1, printpos, scrollingtext[printpos - internalpos]);
          ++printpos;
        }
        internalpos = printpos;
      }
    }
    else if (ui->legendMode() == LEGEND_STATIC) {
      if (staticcount++ > 20) { // 5 seconds
        staticcount = 0;
        if (text.length() - offset > col - 8) {
          size_t nextoffset = text.rfind(" - ", offset + col - 8);
          if (nextoffset != std::string::npos) {
            offset = nextoffset + 3;
          }
          else {
            offset += col - 8;
          }
        }
        else if (offset) {
          offset = 0;
        }
      }
      unsigned int textpos = offset;
      while (printpos < col - 4) {
        if (textpos >= text.length()) {
          ui->printChar(ui->getLegendWindow(), 1, printpos, ' ');
        }
        else {
          ui->printChar(ui->getLegendWindow(), 1, printpos, text[textpos]);
        }
        ++printpos;
        ++textpos;
      }
    }
  }
  return true;
}

void LegendPrinterKeybinds::setText(const std::string & text) {
  this->text = text;
  offset = 0;
  staticcount = 0;
}
