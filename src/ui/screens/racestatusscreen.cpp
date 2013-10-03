#include "racestatusscreen.h"

#include "../../race.h"
#include "../../siterace.h"
#include "../../file.h"
#include "../../filelist.h"
#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../ftpconn.h"
#include "../../engine.h"

#include "../uicommunicator.h"
#include "../termint.h"

extern GlobalContext * global;

RaceStatusScreen::RaceStatusScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  release = uicommunicator->getArg1();
  race = global->getEngine()->getRace(release);
  sitestr = "";
  autoupdate = true;
  spaceous = false;
  smalldirs = false;
  currnumsubpaths = 0;
  currguessedsize = 0;
  for (std::list<SiteLogic *>::iterator it = race->begin(); it != race->end(); it++) {
    sitestr += (*it)->getSite()->getName() + ",";
  }
  if (sitestr.length() > 0) {
    sitestr = sitestr.substr(0, sitestr.length() - 1);
  }
  uicommunicator->checkoutCommand();
  init(window, row, col);
}

void RaceStatusScreen::redraw() {
  werase(window);
  spaceous = false;
  TermInt::printStr(window, 1, 1, "Section: " + race->getSection());
  TermInt::printStr(window, 1, 20, "Sites: " + sitestr);
  std::list<std::string> currsubpaths = race->getSubPaths();
  currnumsubpaths = currsubpaths.size();
  currsubpaths.sort();
  std::string subpathpresent = "";
  subpaths.clear();
  unsigned int sumguessedsize = 0;
  for (std::list<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    if (subpathpresent.length() > 0) {
      subpathpresent += ", ";
    }
    int guessedsize = race->guessedSize(*it);
    bool sfvreported = race->SFVReported(*it);
    std::string pathshow = *it;
    if (pathshow == "") {
      pathshow = "/";
    }
    subpathpresent += pathshow + " (" + global->int2Str(guessedsize) + "f";
    if (sfvreported) {
      subpathpresent += "/sfv";
    }
    subpathpresent += ")";
    if (guessedsize >= 5 || sfvreported) {
      subpaths.push_back(*it);
    }
    sumguessedsize += guessedsize;
  }
  currguessedsize = sumguessedsize;
  TermInt::printStr(window, 2, 1, "Subpaths: " + subpathpresent);
  int y = 4;
  longestsubpath = 0;
  std::list<std::string> filenames;
  for (std::list<std::string>::iterator it = subpaths.begin(); it != subpaths.end(); it++) {
    if (it->length() > longestsubpath) {
      longestsubpath = it->length();
    }
  }
  for (std::list<std::string>::iterator it = subpaths.begin(); it != subpaths.end(); it++) {
    race->prepareGuessedFileList(*it);
    for (std::list<std::string>::iterator it = race->guessedFileListBegin(); it != race->guessedFileListEnd(); it++) {
      std::string filename = *it;
      while (filename.length() < 3) {
        filename += " ";
      }
      std::string lastchars = filename.substr(filename.length() - 3);
      bool duplicate = false;
      for (std::list<std::string>::iterator it3 = filenames.begin(); it3 != filenames.end(); it3++) {
        std::string lastchars2 = *it3;
        if (lastchars2.length() < 3) lastchars += " ";
        lastchars2 = lastchars2.substr(lastchars2.length() - 3);
        if (lastchars == lastchars2) {
          duplicate = true;
          break;
        }
      }
      if (!duplicate) {
        filenames.push_back(filename);
      }
    }
  }
  filenames.sort();
  for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
    if (it->length() > 3 && it->substr(it->length() - 3) == "rar") {
      filenames.push_front(*it);
      filenames.erase(it);
      break;
    }
  }
  for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
    if (it->length() > 3 && it->substr(it->length() - 3) == "sfv") {
      filenames.push_front(*it);
      filenames.erase(it);
      break;
    }
  }
  for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
    if (it->length() > 3 && it->substr(it->length() - 3) == "nfo") {
      filenames.push_front(*it);
      filenames.erase(it);
      break;
    }
  }
  if (longestsubpath > 5) {
    longestsubpath = 5;
  }
  else if (longestsubpath == 0) {
    longestsubpath++;
  }
  int tagx = 8 + longestsubpath;
  if (filenames.size() * 2 + 10 < col) {
    spaceous = true;
  }
  filetagpos.clear();
  for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
    std::string filename = *it;
    while (filename.length() < 3) {
      filename += " ";
    }
    std::string lastchars = filename.substr(filename.length() - 3);
    if (filetagpos.find(lastchars) == filetagpos.end()) {
      filetagpos[lastchars] = tagx;
    }
    TermInt::printStr(window, y, tagx, lastchars.substr(0, 1));
    TermInt::printStr(window, y+1, tagx, lastchars.substr(1, 1));
    TermInt::printStr(window, y+2, tagx++, lastchars.substr(2));
    if (spaceous) {
      tagx++;
    }
  }
  update();
}

void RaceStatusScreen::update() {
  std::list<std::string> currsubpaths = race->getSubPaths();
  unsigned int sumguessedsize = 0;
  for (std::list<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    sumguessedsize += race->guessedSize(*it);;
  }
  if (currsubpaths.size() != currnumsubpaths || sumguessedsize != currguessedsize) {
    redraw();
    return;
  }
  int x = 1;
  int y = 8;
  for (std::list<SiteLogic *>::iterator it = race->begin(); it != race->end(); it++) {
    SiteRace * sr = (*it)->getRace(release);
    std::string user = (*it)->getSite()->getUser();
    TermInt::printStr(window, y, x, (*it)->getSite()->getName(), 4);
    for (std::list<std::string>::iterator it2 = subpaths.begin(); it2 != subpaths.end(); it2++) {
      std::string origsubpath = *it2;
      if (!smalldirs && race->guessedSize(origsubpath) < 5) {
        continue;
      }
      std::string printsubpath = origsubpath;
      FileList * fl = sr->getFileListForPath(origsubpath);
      if (printsubpath == "") {
        printsubpath = "/";
      }

      TermInt::printStr(window, y, x + 5, printsubpath, longestsubpath);
      race->prepareGuessedFileList(origsubpath);
      for (std::list<std::string>::iterator it3 = race->guessedFileListBegin(); it3 != race->guessedFileListEnd(); it3++) {
        std::string filename = *it3;
        if (filename.length() < 3) filename += " ";
        int filex = filetagpos[filename.substr(filename.length() - 3)];
        File * file;
        char printchar = '_';
        bool highlight = false;
        bool upload = false;
        bool download = false;
        bool owner = false;
        if ((file = fl->getFile(*it3)) != NULL) {
          highlight = true;
          if (file->isUploading() || file->getSize() < race->guessedFileSize(origsubpath, *it3)) {
            upload = true;
          }
          if (file->getOwner() == user) {
            owner = true;
          }
          if (file->isDownloading()) {
            download = true;
          }
          printchar = getFileChar(owner, upload, download);
        }
        if (highlight) wattron(window, A_REVERSE);
        TermInt::printChar(window, y, filex, printchar);
        if (highlight) wattroff(window, A_REVERSE);
      }
      y++;
    }
  }
}

void RaceStatusScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case ' ':
    case 10:
      uicommunicator->newCommand("return");
      break;
    case 's':
      if (smalldirs) {
        smalldirs = false;
      }
      else {
        smalldirs = true;
      }
      uicommunicator->newCommand("redraw");
      break;
  }
}

char RaceStatusScreen::getFileChar(bool owner, bool upload, bool download) {
  char printchar = '_';
  if (upload) {
    if (owner) {
      if (download) {
        printchar = 'S';
      }
      else {
        printchar = 'U';
      }
    }
    else {
      if (download) {
        printchar = 's';
      }
      else {
        printchar = 'u';
      }
    }
  }
  else {
    if (owner) {
      if (download) {
        printchar = 'D';
      }
      else {
        printchar = 'o';
      }
    }
    else {
      if (download) {
        printchar = 'd';
      }
    }
  }
  return printchar;
}

std::string RaceStatusScreen::getLegendText() {
  return "[Enter] Return - [R]emove site from race - [A]dd site to race - A[B]ort race - [s]how small dirs";
}

std::string RaceStatusScreen::getInfoLabel() {
  return "RACE STATUS: " + release;
}
