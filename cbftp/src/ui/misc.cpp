#include "misc.h"

#include "termint.h"
#include "ui.h"

void printSlider(Ui * ui, unsigned int row, unsigned int xpos, unsigned int totalspan, unsigned int currentviewspan) {
  printSlider(ui, row, 0, xpos, totalspan, currentviewspan);
}

void printSlider(Ui * ui, unsigned int row, unsigned int ypos, unsigned int xpos, unsigned int totalspan, unsigned int currentviewspan) {
  unsigned int slidersize = 0;
  unsigned int sliderstart = 0;
  unsigned int spanlength = row - ypos;
  if (currentviewspan + spanlength > totalspan) {
    totalspan = currentviewspan + spanlength;
  }
  if (totalspan > spanlength) {
    slidersize = (spanlength * spanlength) / totalspan;
    sliderstart = (spanlength * currentviewspan) / totalspan;
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == spanlength) {
      slidersize--;
    }
    if (sliderstart + slidersize > spanlength || currentviewspan + spanlength >= totalspan) {
      sliderstart = spanlength - slidersize;
    }
    for (unsigned int i = 0; i < spanlength; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        ui->printChar(i + ypos, xpos, ' ', true);
      }
      else {
        ui->printChar(i + ypos, xpos, BOX_VLINE);
      }
    }
    if (spanlength == 1) {
      ui->printChar(ypos, xpos, BOX_CROSS);
    }
  }
}

bool adaptViewSpan(unsigned int & currentviewspan, unsigned int row, unsigned int position, unsigned int listsize) {
  unsigned int pagerows = row / 2;
  bool viewspanchanged = false;
  if (position < currentviewspan || position >= currentviewspan + row) {
    if (position < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = position - pagerows;
    }
    viewspanchanged = true;
  }
  if (currentviewspan + row > listsize && listsize >= row) {
    currentviewspan = listsize - row;
    if (currentviewspan > position) {
      currentviewspan = position;
    }
    viewspanchanged = true;
  }
  return viewspanchanged;
}
