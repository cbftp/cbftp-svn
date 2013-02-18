#include "racestatusscreen.h"

RaceStatusScreen::RaceStatusScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  release = uicommunicator->getArg1();
  race = global->getEngine()->getRace(release);
  sitestr = "";
  autoupdate = true;
  for (std::list<SiteThread *>::iterator it = race->begin(); it != race->end(); it++) {
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
  TermInt::printStr(window, 1, 1, "Race status for " + release);
  TermInt::printStr(window, 2, 1, "Section: " + race->getSection());
  TermInt::printStr(window, 2, 20, "Sites: " + sitestr);
  std::list<std::string> currsubpaths = race->getSubPaths();
  subpaths.clear();
  for (std::list<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    if (race->guessedSize(*it) >= 5 || race->SFVReported(*it)) {
      subpaths.push_back(*it);
    }
  }
  subpaths.sort();
  bool listed = false;
  int y = 4;
  int x = 1;
  unsigned int longestsubpath = 0;
  bool pathlencheck = false;
  std::list<std::string> filenames;
  for (std::list<SiteThread *>::iterator it = race->begin(); it != race->end(); it++) {
    SiteRace * sr = (*it)->getRace(release);
    for (std::list<std::string>::iterator it2 = subpaths.begin(); it2 != subpaths.end(); it2++) {
      if (!pathlencheck && it2->length() > longestsubpath) {
        longestsubpath = it2->length();
      }
      if (!listed) {
        FileList * fl = sr->getFileListForPath(*it2);
        if (fl != NULL) {
          for (std::map<std::string, File *>::iterator it = fl->begin(); it != fl->end(); it++) {
            if (!it->second->isDirectory()) {
              std::string filename = it->first;
              size_t dotpos = filename.rfind(".");
              if (dotpos != std::string::npos && filename.length() > dotpos + 3) {
                filename = filename.substr(0, dotpos + 4);
              }
              filenames.push_back(filename);
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
          filetagpos.clear();
          int pos = 0;
          for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
            filetagpos[*it] = pos++;
          }
          listed = true;
        }
      }
    }
    pathlencheck = true;
  }
  if (longestsubpath > 5) {
    longestsubpath = 5;
  }
  else if (longestsubpath == 0) {
    longestsubpath++;
  }
  int tagx = 8 + longestsubpath;
  for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {
    std::string lastchars = it->substr(it->length() - 3);
    TermInt::printStr(window, y, tagx, lastchars.substr(0, 1));
    TermInt::printStr(window, y+1, tagx, lastchars.substr(1, 1));
    TermInt::printStr(window, y+2, tagx++, lastchars.substr(2));
  }
  y = y + 4;
  for (std::list<SiteThread *>::iterator it = race->begin(); it != race->end(); it++) {
    TermInt::printStr(window, y, x, (*it)->getSite()->getName(), 4);
    for (std::list<std::string>::iterator it2 = subpaths.begin(); it2 != subpaths.end(); it2++) {
      std::string subpath = *it2;
      if (subpath == "") {
        subpath = "/";
      }
      TermInt::printStr(window, y++, x + 5, subpath, longestsubpath);
    }
  }
  update();
}

void RaceStatusScreen::update() {
  std::list<std::string> currsubpaths = race->getSubPaths();
  std::list<std::string> carepaths;
  for (std::list<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    if (race->guessedSize(*it) >= 5 || race->SFVReported(*it)) {
      carepaths.push_back(*it);
    }
  }
  carepaths.sort();
  if (carepaths != subpaths) {
    subpaths = carepaths;
    redraw();
    return;
  }
}

void RaceStatusScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case ' ':
    case 10:
      uicommunicator->newCommand("return");
      break;
  }
}

std::string RaceStatusScreen::getLegendText() {
  return "[Enter] Return";
}
