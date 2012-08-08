#include "browsescreen.h"

BrowseScreen::BrowseScreen(WINDOW * window, UICommunicator * uicommunicator, int row, int col) {
  this->uicommunicator = uicommunicator;
  sitethread = global->getSiteThreadManager()->getSiteThread(uicommunicator->getArg1());
  site = sitethread->getSite();
  uicommunicator->expectBackendPush();
  requestid = sitethread->requestFileList("/");
  init(window, row, col);
}

void BrowseScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "Let's browse this shit!");
  TermInt::printStr(window, 2, 1, "Fetching list...");
}

void BrowseScreen::update() {
  if (sitethread->requestReady(requestid)) {
    //list.parse(sitethread->getFileList(requestid));
    TermInt::printStr(window, 4, 1, "LIST FETCHED!");
  }
}

void BrowseScreen::keyPressed(int ch) {
  switch (ch) {
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'r':
      //start a race of the selected dir, do nothing if a file is selected
      break;
    case 'b':
      //bind selected dir to section, do nothing if a file is selected
      break;
    case 'v':
      //view selected file, do nothing if a directory is selected
      break;
    case KEY_RIGHT:
    case 10:
      //sitethread->requestFileList(currentpath + list.selected());
      //enter the selected directory, do nothing if a file is selected
      uicommunicator->newCommand("update");
      break;
    case KEY_LEFT:
    case KEY_BACKSPACE:
      uicommunicator->newCommand("update");
      //go up one directory level, or return if at top already
      break;
    case KEY_DOWN:
      //go down and highlight next item (if not at bottom already)
      uicommunicator->newCommand("update");
      break;
    case KEY_UP:
      //go up and highlight previous item (if not at top already)
      uicommunicator->newCommand("update");
      break;
  }
}

std::string BrowseScreen::getLegendText() {
  return "[c]ancel - [Enter/Right] open dir - [Backspace/Left] return - [r]ace - [v]iew file - [b]ind to section";
}
