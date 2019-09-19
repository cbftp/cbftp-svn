#include "racestatusscreen.h"

#include <cassert>
#include <set>
#include <unordered_map>
#include <unordered_set>

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
#include "../../sitemanager.h"
#include "../../util.h"

#include "../ui.h"
#include "../menuselectoptiontextbutton.h"

namespace {

enum SelectSitesMode {
  SELECT_ADD,
  SELECT_DELETE,
  SELECT_DELETE_OWN
};

char getFileChar(bool exists, bool owner, bool upload, bool download, bool affil) {
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
      else if (exists) {
        if (affil) {
          printchar = 'p';
        }
        else {
          printchar = '.';
        }
      }
    }
  }
  return printchar;
}

}

RaceStatusScreen::RaceStatusScreen(Ui * ui) {
  this->ui = ui;
  defaultlegendtext = "[c/Esc] Return - [Del] Remove site from job - [A]dd site to job - [s]how small dirs - [r]eset job - Hard [R]eset job - A[B]ort job - [d]elete site and own files from job - [D]elete site and all files from job - [t]transfers - [T]ransfers for site - [b]rowse - [E]dit site - [z] Abort job and delete own files on incomplete sites";
  finishedlegendtext = "[c/Esc] Return - [Del] Remove site from job - [A]dd site to job - [s]how small dirs - [r]eset job - Hard [R]eset job - [d]elete own files - [D]elete all files - [t]ransfers - [T]ransfers for site - [b]rowse - [E]dit site - [z] Delete own files on all sites";
}

RaceStatusScreen::~RaceStatusScreen() {

}

void RaceStatusScreen::initialize(unsigned int row, unsigned int col, unsigned int id) {
  race = global->getEngine()->getRace(id);
  if (race->getStatus() == RaceStatus::RUNNING) {
    finished = false;
  }
  else {
    finished = true;
  }
  autoupdate = true;
  smalldirs = false;
  awaitingremovesite = false;
  awaitingremovesitedelownfiles = false;
  awaitingremovesitedelallfiles = false;
  awaitingabort = false;
  awaitingdeleteownall = false;
  currnumsubpaths = 0;
  currguessedsize = 0;
  currincomplete = 0;
  mso.enterFocusFrom(0);
  init(row, col);
}

void RaceStatusScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "Section: " + race->getSection());
  ui->printStr(1, 20, "Sites: " + race->getSiteListText());
  std::list<std::string> incompletesites = getIncompleteSites();
  if (!incompletesites.empty()) {
    ui->printStr(3, 18, "Incomplete on: " + util::join(incompletesites, ","));
  }
  currincomplete = incompletesites.size();
  std::unordered_set<std::string> currsubpaths = race->getSubPaths();
  currnumsubpaths = currsubpaths.size();
  std::string subpathpresent = "";
  subpaths.clear();
  unsigned int sumguessedsize = 0;
  for (std::unordered_set<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    if (subpathpresent.length() > 0) {
      subpathpresent += ", ";
    }
    int guessedsize = race->guessedSize(*it);
    bool sfvreported = race->SFVReported(*it);
    std::string pathshow = *it;
    if (pathshow == "") {
      pathshow = "/";
    }
    subpathpresent += pathshow + " (" + std::to_string(guessedsize) + "f";
    if (sfvreported) {
      subpathpresent += "/sfv";
    }
    subpathpresent += ")";
    if (pathshow == "/" || guessedsize >= 5 || sfvreported) {
      subpaths.push_back(*it);
    }
    sumguessedsize += guessedsize;
  }
  currguessedsize = sumguessedsize;
  ui->printStr(2, 1, "Subpaths: " + subpathpresent);
  int y = 5;
  longestsubpath = 0;
  std::list<std::string> filetags;
  for (std::list<std::string>::iterator it = subpaths.begin(); it != subpaths.end(); it++) {
    if (it->length() > longestsubpath) {
      longestsubpath = it->length();
    }
  }
  std::set<std::string> bannedsuffixes;
  std::map<std::string, std::string> tags;
  filenametags.clear();
  for (std::list<std::string>::iterator subit = subpaths.begin(); subit != subpaths.end(); subit++) {
    bool finished = false;
    std::map<std::string, std::string> localtags;
    while (!finished) {
      finished = true;
      for (std::unordered_map<std::string, unsigned long long int>::const_iterator it =
          race->guessedFileListBegin(*subit); it != race->guessedFileListEnd(*subit); it++) {
        std::string filename = it->first;
        while (filename.length() < 3) {
          filename += " ";
        }
        std::string tag = filename.substr(filename.length() - 3); // first tag attempt, last three chars
        if (bannedsuffixes.find(tag) != bannedsuffixes.end()) {
          tag = filename.substr(0, 3);                            // second tag attempt, first three chars
          if (bannedsuffixes.find(tag) != bannedsuffixes.end()) {
            size_t dotpos = filename.rfind(".");
            if (dotpos == std::string::npos) {
              dotpos = filename.length() - 1;
            }
            for (unsigned int i = 3; i <= dotpos && bannedsuffixes.find(tag) != bannedsuffixes.end(); i++) {
              tag = filename.substr(dotpos - i, 3);               // many tag attempts stepping from the end
            }
            if (bannedsuffixes.find(tag) != bannedsuffixes.end()) {
              for (int i = 0; i < 100; i++) {
                std::string numtag = std::to_string(i);
                while (numtag.length() < 2) {
                  numtag = "0" + numtag;
                }
                tag = filename[filename.length() - 3] + numtag;
                if (bannedsuffixes.find(tag) == bannedsuffixes.end() && localtags.find(tag) == localtags.end()) {
                  break;
                }
                if (i == 99) {
                  for (int i = 0; i < 1000; i++) { // last resort
                    tag = std::to_string(i);
                    while (tag.length() < 3) {
                      tag = "0" + tag;
                    }
                    if (bannedsuffixes.find(tag) == bannedsuffixes.end() && localtags.find(tag) == localtags.end()) {
                      break;
                    }
                    if (i == 999) {
                      assert(false); // whatever, this should never happen
                    }
                  }
                }
              }
            }
          }
        }
        if (localtags.find(tag) != localtags.end()) {
          bannedsuffixes.insert(tag);
          localtags.clear();
          finished = false;
          break;
        }
        localtags[tag] = filename;
      }
    }
    for (std::map<std::string, std::string>::iterator it = localtags.begin(); it != localtags.end(); it++) {
      tags[it->first] = it->second;
      filenametags[it->second] = it->first;
    }
  }
  for (std::map<std::string, std::string>::iterator it = tags.begin(); it != tags.end(); it++) {
    filetags.push_back(it->first);
  }
  filetags.sort();
  for (std::list<std::string>::iterator it = filetags.begin(); it != filetags.end(); it++) {
    if (*it == "rar") {
      filetags.push_front(*it);
      filetags.erase(it);
      break;
    }
  }
  for (std::list<std::string>::iterator it = filetags.begin(); it != filetags.end(); it++) {
    if (*it == "sfv") {
      filetags.push_front(*it);
      filetags.erase(it);
      break;
    }
  }
  for (std::list<std::string>::iterator it = filetags.begin(); it != filetags.end(); it++) {
    if (*it == "nfo") {
      filetags.push_front(*it);
      filetags.erase(it);
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
  filetagpos.clear();
  for (std::list<std::string>::iterator it = filetags.begin(); it != filetags.end(); it++) {
    std::string tag = *it;
    filetagpos[tag] = tagx;
    ui->printStr(y, tagx, tag.substr(0, 1));
    ui->printStr(y+1, tagx, tag.substr(1, 1));
    ui->printStr(y+2, tagx++, tag.substr(2));
  }
  update();
}

void RaceStatusScreen::update() {
  if (!finished) {
    if (race->getStatus() != RaceStatus::RUNNING) {
      finished = true;
      ui->setLegend();
    }
  }
  std::unordered_set<std::string> currsubpaths = race->getSubPaths();
  unsigned int sumguessedsize = 0;
  bool haslargepath = false;
  for (std::unordered_set<std::string>::iterator it = currsubpaths.begin(); it != currsubpaths.end(); it++) {
    unsigned int guessedsize = race->guessedSize(*it);
    sumguessedsize += guessedsize;
    if (guessedsize >= 5) {
      haslargepath = true;
    }
  }

  if (currsubpaths.size() != currnumsubpaths || sumguessedsize != currguessedsize || getIncompleteSites().size() != currincomplete) {
    redraw();
    return;
  }
  std::string status;
  switch (race->getStatus()) {
    case RaceStatus::RUNNING:
      status = "Running";
      break;
    case RaceStatus::DONE:
      status = "Done   ";
      break;
    case RaceStatus::ABORTED:
      status = "Aborted";
      break;
    case RaceStatus::TIMEOUT:
      status = "Timeout";
      break;
  }
  ui->printStr(3, 1, "Status: " + status);
  int x = 1;
  int y = 8;
  mso.clear();
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
    const std::shared_ptr<SiteRace> & sr = it->first;
    const std::shared_ptr<SiteLogic> & sl = it->second;
    std::string user = sl->getSite()->getUser();
    bool trimcompare = user.length() > 8;
    std::string trimuser = user;
    if (trimcompare) {
      trimuser = user.substr(0, 8);
    }
    std::string sitename = sl->getSite()->getName();
    bool affil = sl->getSite()->isAffiliated(race->getGroup());
    mso.addTextButton(y, x, sitename, sitename);
    for (std::list<std::string>::iterator it2 = subpaths.begin(); it2 != subpaths.end(); it2++) {
      std::string origsubpath = *it2;
      if (haslargepath && !smalldirs && race->guessedSize(origsubpath) < 5) {
        continue;
      }
      std::string printsubpath = origsubpath;
      std::shared_ptr<FileList> fl = sr->getFileListForPath(origsubpath);
      if (!fl) {
        continue;
      }
      if (printsubpath == "") {
        printsubpath = "/";
      }

      ui->printStr(y, x + 5, printsubpath, false, longestsubpath);
      for (std::unordered_map<std::string, unsigned long long int>::const_iterator it3 =
          race->guessedFileListBegin(origsubpath); it3 != race->guessedFileListEnd(origsubpath); it3++) {
        std::string filename = it3->first;
        if (filename.length() < 3) filename += " ";
        int filex = filetagpos[filenametags[filename]];
        File * file;
        char printchar = '_';
        bool exists = false;
        bool upload = false;
        bool download = false;
        bool owner = false;
        if ((file = fl->getFile(filename)) != NULL) {
          exists = true;
          if (file->isUploading() || (file->getSize() < race->guessedFileSize(origsubpath, filename) &&
                                      !sr->isSubPathComplete(fl))) {
            upload = true;
          }
          std::string ownerstr = file->getOwner();
          if (ownerstr == user || (trimcompare && ownerstr == trimuser)) {
            owner = true;
          }
          if (file->isDownloading()) {
            download = true;
          }
          printchar = getFileChar(exists, owner, upload, download, affil);
        }
        ui->printChar(y, filex, printchar, exists);
      }
      y++;
    }
  }
  mso.checkPointer();
  unsigned int selected = mso.getSelectionPointer();
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(i));
    bool isselected = selected == i;
    ui->printStr(msotb->getRow(), msotb->getCol(), msotb->getLabelText(), isselected, 4);
  }
}

void RaceStatusScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (awaitingremovesite) {
      global->getEngine()->removeSiteFromRace(race, removesite);
    }
    else if (awaitingremovesitedelownfiles) {
      global->getEngine()->removeSiteFromRaceDeleteFiles(race, removesite, false);
    }
    else if (awaitingremovesitedelallfiles) {
      global->getEngine()->removeSiteFromRaceDeleteFiles(race, removesite, true);
    }
    else if (awaitingabort) {
      global->getEngine()->abortRace(race);
      finished = true;
      ui->setLegend();
    }
    else if (awaitingdeleteownall) {
      global->getEngine()->deleteOnAllIncompleteSites(race, false);
    }
  }
  else if (command == "returnselectitems") {
    std::string preselectstr = arg;
    std::list<std::shared_ptr<Site> > selectedsites;
    while (true) {
      size_t commapos = preselectstr.find(",");
      if (commapos != std::string::npos) {
        std::string sitename = preselectstr.substr(0, commapos);
        std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
        selectedsites.push_back(site);
        preselectstr = preselectstr.substr(commapos + 1);
      }
      else {
        if (preselectstr.length() > 0) {
          std::shared_ptr<Site> site = global->getSiteManager()->getSite(preselectstr);
          selectedsites.push_back(site);
        }
        break;
      }
    }
    if (selectsitesmode == SELECT_ADD) {
      for (std::list<std::shared_ptr<Site> >::iterator it = selectedsites.begin(); it != selectedsites.end(); it++) {
        global->getEngine()->addSiteToRace(race, (*it)->getName());
      }
    }
    else if (selectsitesmode == SELECT_DELETE) {
      global->getEngine()->deleteOnSites(race, selectedsites, true);
    }
    else if (selectsitesmode == SELECT_DELETE_OWN) {
      global->getEngine()->deleteOnSites(race, selectedsites, false);
    }
  }
  awaitingremovesite = false;
  awaitingremovesitedelownfiles = false;
  awaitingremovesitedelallfiles = false;
  awaitingabort = false;
  awaitingdeleteownall = false;
  ui->redraw();
}

bool RaceStatusScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 's':
      if (smalldirs) {
        smalldirs = false;
      }
      else {
        smalldirs = true;
      }
      ui->redraw();
      return true;
    case KEY_UP:
      if (mso.goUp()) {
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (mso.goDown()) {
        ui->update();
      }
      return true;
    case KEY_DC:
    {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
      if (!!msotb) {
        removesite = msotb->getLabelText();
        awaitingremovesite = true;
        ui->goConfirmation("Do you really want to remove " + removesite + " from the race?");
      }
      return true;
    }
    case 'B':
      if (race->getStatus() == RaceStatus::RUNNING) {
        awaitingabort = true;
        ui->goConfirmation("Do you really want to abort the race " + race->getName());
      }
      return true;
    case 'd':
      if (race->getStatus() != RaceStatus::RUNNING) {
        deleteFiles(false);
      }
      else {
        std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
        if (!!msotb) {
          removesite = msotb->getLabelText();
          awaitingremovesitedelownfiles = true;
          ui->goConfirmation("Do you really want to remove " + removesite + " from the race and delete own files?");
        }
      }
      return true;
    case 'D':
      if (race->getStatus() != RaceStatus::RUNNING) {
        deleteFiles(true);
      }
      else {
        std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
        if (!!msotb) {
          removesite = msotb->getLabelText();
          awaitingremovesitedelallfiles = true;
          ui->goConfirmation("Do you really want to remove " + removesite + " from the race and delete all files?");
        }
      }
      return true;
    case 'z':
      awaitingdeleteownall = true;
      if (race->getStatus() == RaceStatus::RUNNING) {
        ui->goConfirmation("Do you really want to abort the race " + race->getName() + " and delete your own files on all incomplete sites?");
      }
      else {
        ui->goConfirmation("Do you really want to delete your own files in " + race->getName() + " on all involved sites?");
      }
      return true;
    case 'A':
    {
      std::list<std::shared_ptr<Site> > excludedsites;
      for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
        excludedsites.push_back(it->second->getSite());
      }
      std::vector<std::shared_ptr<Site> >::const_iterator it;
      for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
        if (!(*it)->hasSection(race->getSection()) ||
            ((((*it)->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && !(*it)->isAffiliated(race->getGroup())) ||
              (*it)->getAllowDownload() == SITE_ALLOW_TRANSFER_NO) &&
             (*it)->getAllowUpload() == SITE_ALLOW_TRANSFER_NO) ||
            (*it)->getDisabled())
        {
          excludedsites.push_back(*it);
        }
      }
      selectsitesmode = SELECT_ADD;
      ui->goSelectSites("Add these sites to the race: " + race->getSection() + "/" + race->getName(), std::list<std::shared_ptr<Site> >(), excludedsites);
      return true;
    }
    case 'r':
      global->getEngine()->resetRace(race, false);
      return true;
    case 'R':
      global->getEngine()->resetRace(race, true);
      return true;
    case 't':
      ui->goTransfersFilterSpreadJob(race->getName());
      return true;
    case 'T': {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
      if (!!msotb) {
        std::string site = msotb->getLabelText();
        ui->goTransfersFilterSpreadJobSite(race->getName(), site);
      }
      return true;
    }
    case 10: {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
      if (!!msotb) {
        ui->goSiteStatus(msotb->getLabelText());
      }
      break;
    }
    case 'b': {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
      if (!!msotb) {
        std::string site = msotb->getLabelText();
        ui->goBrowse(site, race->getSiteRace(site)->getPath());
      }
      break;
    }
    case 'E': {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(mso.getElement(mso.getSelectionPointer()));
      if (!!msotb) {
        std::string site = msotb->getLabelText();
        ui->goEditSite(site);
      }
      break;
    }
  }
  return false;
}

void RaceStatusScreen::deleteFiles(bool allfiles) {
  if (race->getStatus() != RaceStatus::RUNNING) {
    std::list<std::shared_ptr<Site> > sites;
    std::list<std::shared_ptr<Site> > preselectsites;
    std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it;
    for (it = race->begin(); it != race->end(); it++) {
      sites.push_back(it->second->getSite());
      if (global->getEngine()->isIncompleteEnoughForDelete(race, it->first)) {
        preselectsites.push_back(it->second->getSite());

      }
    }
    selectsitesmode = allfiles ? SELECT_DELETE : SELECT_DELETE_OWN;
    ui->goSelectSitesFrom(std::string("Delete ") + (allfiles ? "all" : "own") + " files in " + race->getName() + " from these sites", preselectsites, sites);
  }
}

std::string RaceStatusScreen::getLegendText() const {
  return finished ? finishedlegendtext : defaultlegendtext;
}

std::string RaceStatusScreen::getInfoLabel() const {
  return "RACE STATUS: " + race->getName();
}

std::list<std::string> RaceStatusScreen::getIncompleteSites() const {
  std::list<std::string> incompletesites;
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
    if (it->first->getStatus() != RaceStatus::DONE) {
      incompletesites.push_back(it->first->getSiteName());
    }
  }
  return incompletesites;
}
