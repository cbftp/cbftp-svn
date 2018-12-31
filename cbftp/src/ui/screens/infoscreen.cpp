#include "infoscreen.h"

#include "../../globalcontext.h"
#include "../../statistics.h"
#include "../../util.h"
#include "../../buildinfo.h"

#include "../../core/sslmanager.h"

#include "../ui.h"

InfoScreen::InfoScreen(Ui * ui) {
  this->ui = ui;
}

void InfoScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  init(row, col);
}

void InfoScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  unsigned long long int sizefxpday = global->getStatistics()->getSizeFXPLast24Hours();
  unsigned int filesfxpday = global->getStatistics()->getFilesFXPLast24Hours();
  unsigned long long int sizefxpall = global->getStatistics()->getSizeFXPAll();
  unsigned int filesfxpall = global->getStatistics()->getFilesFXPAll();
  unsigned long long int sizeupday = global->getStatistics()->getSizeUpLast24Hours();
  unsigned int filesupday = global->getStatistics()->getFilesUpLast24Hours();
  unsigned long long int sizeupall = global->getStatistics()->getSizeUpAll();
  unsigned int filesupall = global->getStatistics()->getFilesUpAll();
  unsigned long long int sizedownday = global->getStatistics()->getSizeDownLast24Hours();
  unsigned int filesdownday = global->getStatistics()->getFilesDownLast24Hours();
  unsigned long long int sizedownall = global->getStatistics()->getSizeDownAll();
  unsigned int filesdownall = global->getStatistics()->getFilesDownAll();

  unsigned int i = 1;
  ui->printStr(i++, 1, "Compile time: " + BuildInfo::compileTime());
  ui->printStr(i++, 1, "Version tag: " + BuildInfo::version());
  ui->printStr(i++, 1, "Distribution tag: " + BuildInfo::tag());
  ui->printStr(i++, 1, "OpenSSL version: " + SSLManager::version());
  i++;
  ui->printStr(i++, 1, "Traffic measurements");
  ui->printStr(i++, 1, "FXP      last 24 hours: " + util::parseSize(sizefxpday) + ", " +
               std::to_string(filesfxpday) + " files - All time: " + util::parseSize(sizefxpall) + ", " +
               std::to_string(filesfxpall) + " files");
  ui->printStr(i++, 1, "Upload   last 24 hours: " + util::parseSize(sizeupday) + ", " +
                 std::to_string(filesupday) + " files - All time: " + util::parseSize(sizeupall) + ", " +
                 std::to_string(filesupall) + " files");
  ui->printStr(i++, 1, "Download last 24 hours: " + util::parseSize(sizedownday) + ", " +
                 std::to_string(filesdownday) + " files - All time: " + util::parseSize(sizedownall) + ", " +
                 std::to_string(filesdownall) + " files");
  ++i;
  ui->printStr(i++, 1, "All time spread jobs: " + std::to_string(global->getStatistics()->getSpreadJobs()));
  ui->printStr(i++, 1, "All time transfer jobs: " + std::to_string(global->getStatistics()->getTransferJobs()));
}

void InfoScreen::update() {
  redraw();
}

bool InfoScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string InfoScreen::getLegendText() const {
  return "[Any] Return";
}

std::string InfoScreen::getInfoLabel() const {
  return "GENERAL INFO";
}
