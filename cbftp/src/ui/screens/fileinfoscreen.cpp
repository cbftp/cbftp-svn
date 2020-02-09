#include "fileinfoscreen.h"

#include "../../util.h"

#include "../ui.h"
#include "../uifile.h"

FileInfoScreen::FileInfoScreen(Ui* ui) : uifile(nullptr) {
  this->ui = ui;
}

void FileInfoScreen::initialize(unsigned int row, unsigned int col, UIFile* uifile) {
  this->uifile = uifile;
  init(row, col);
}

void FileInfoScreen::redraw() {
  ui->erase();
  ui->hideCursor();

  unsigned int i = 1;
  ui->printStr(i++, 1, "Name: " + uifile->getName());
  std::string type = "File";
  if (uifile->isDirectory()) {
    type = "Directory";
  }
  else if (uifile->isLink()) {
    type = "Link";
  }
  ui->printStr(i++, 1, "Type: " + type);
  ui->printStr(i++, 1, "Size: " + uifile->getSizeRepr());
  ui->printStr(i++, 1, "Last modified: " + uifile->getLastModified());
  ui->printStr(i++, 1, "Owner: " + uifile->getOwner() + "/" + uifile->getGroup());
  if (uifile->isLink()) {
    ui->printStr(i++, 1, "Link target: " + uifile->getLinkTarget());
  }
}

void FileInfoScreen::update() {
  redraw();
}

bool FileInfoScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string FileInfoScreen::getLegendText() const {
  return "[Any] Return";
}

std::string FileInfoScreen::getInfoLabel() const {
  return "FILE INFO";
}
