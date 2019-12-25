#include "browsescreen.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include "../../globalcontext.h"
#include "../../engine.h"
#include "../../localfilelist.h"
#include "../../timereference.h"
#include "../../filelist.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sectionmanager.h"
#include "../../section.h"

#include "../ui.h"
#include "../uifilelist.h"
#include "../termint.h"

#include "browsescreensub.h"
#include "browsescreensite.h"
#include "browsescreenselector.h"
#include "browsescreenaction.h"
#include "browsescreenlocal.h"

BrowseScreen::BrowseScreen(Ui * ui) {
  this->ui = ui;
}

BrowseScreen::~BrowseScreen() {

}

void BrowseScreen::initialize(unsigned int row, unsigned int col, ViewMode viewmode, const std::string & site, const Path path) {
  expectbackendpush = true;
  this->split = initsplitupdate = viewmode == VIEW_SPLIT;
  TimeReference::updateTime();
  if (viewmode != VIEW_LOCAL) {
    left = std::make_shared<BrowseScreenSite>(ui, this, site, path);
  }
  else {
    left = std::make_shared<BrowseScreenLocal>(ui);
  }
  if (split) {
    left->setFocus(false);
    active = right = std::make_shared<BrowseScreenSelector>(ui);
  }
  else {
    right.reset();
    active = left;
  }
  init(row, col);
}

void BrowseScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  ui->setSplit(split);
  unsigned int splitcol = col / 2;
  if (split) {
    for (unsigned int i = 0; i < row; i++) {
      ui->printChar(i, splitcol, BOX_VLINE);
    }
  }
  if (!!left) {
    if (!split) {
      left->redraw(row, col, 0);

    }
    else {
      left->redraw(row, splitcol, 0);
    }
  }
  if (!!right) {
    if (!split) {
      right->redraw(row, col, 0);
    }
    else {
      right->redraw(row, col - splitcol - 1, splitcol + 1);
    }
  }
}

void BrowseScreen::update() {
  if (!!left) {
    left->update();
  }
  if (!!right) {
    right->update();
  }
}

void BrowseScreen::command(const std::string & command, const std::string & arg) {
  active->command(command, arg);
}

bool BrowseScreen::keyPressed(unsigned int ch) {
  BrowseScreenAction op = active->keyPressed(ch);
  switch (op.getOp()) {
    case BROWSESCREENACTION_CLOSE:
      clearCompareListMode();
      if (active->type() != BrowseScreenType::SELECTOR && split) {
        if (active == left) {
          if (right->type() == BrowseScreenType::SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = left = std::make_shared<BrowseScreenSelector>(ui);
            ui->redraw();
            ui->setInfo();
            ui->setLegend();
            return true;
          }
        }
        else {
          if (left->type() == BrowseScreenType::SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = right = std::make_shared<BrowseScreenSelector>(ui);
            ui->redraw();
            ui->setInfo();
            ui->setLegend();
            return true;
          }
        }
      }
      else {
        closeSide();
      }
      return true;
    case BROWSESCREENACTION_SITE: {
      Path targetpath;
      if (split) {
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        if (other->type() == BrowseScreenType::SITE) {
          const std::shared_ptr<BrowseScreenSite> & othersite = std::static_pointer_cast<BrowseScreenSite>(other);
          const Path & path = othersite->getUIFileList()->getPath();
          std::list<std::string> sections = othersite->getSite()->getSectionsForPath(path);
          if (!sections.empty()) {
            std::string section = sections.front();
            targetpath = global->getSiteManager()->getSite(op.getArg())->getSectionPath(section);
          }
        }
      }
      active = (active == left ? left : right) = std::make_shared<BrowseScreenSite>(ui, this, op.getArg(), targetpath);
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    }
    case BROWSESCREENACTION_HOME:
      active = (active == left ? left : right) = std::make_shared<BrowseScreenLocal>(ui);
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    case BROWSESCREENACTION_NOOP:
      return keyPressedNoSubAction(ch);
    case BROWSESCREENACTION_CAUGHT:
      return true;
    case BROWSESCREENACTION_CHDIR:
      clearCompareListMode();
      return true;
  }
  return false;
}

bool BrowseScreen::keyPressedNoSubAction(unsigned int ch) {
  switch (ch) {
    case '\t':
      if (!split) {
        split = true;
        right = std::make_shared<BrowseScreenSelector>(ui);
      }
      {
        switchSide();
      }
      return true;
    case 't':
      if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        if (active->type() == BrowseScreenType::SITE) {
          std::shared_ptr<FileList> activefl = std::static_pointer_cast<BrowseScreenSite>(active)->fileList();
          std::list<UIFile *> files = std::static_pointer_cast<BrowseScreenSite>(active)->getUIFileList()->getSelectedFiles();
          if (activefl != NULL) {
            if (other->type() == BrowseScreenType::SITE) {
              std::shared_ptr<FileList> otherfl = std::static_pointer_cast<BrowseScreenSite>(other)->fileList();
              if (otherfl != NULL) {
                for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
                  UIFile * f = *it;
                  if (!f || (!f->isDirectory() && !f->getSize())) {
                    continue;
                  }
                  unsigned int id = global->getEngine()->newTransferJobFXP(std::static_pointer_cast<BrowseScreenSite>(active)->siteName(),
                                                                           activefl,
                                                                           std::static_pointer_cast<BrowseScreenSite>(other)->siteName(),
                                                                           otherfl,
                                                                           f->getName());
                  ui->addTempLegendTransferJob(id);
                }
              }
            }
            else {
              std::shared_ptr<LocalFileList> otherfl = std::static_pointer_cast<BrowseScreenLocal>(other)->fileList();
              if (!!otherfl) {
                for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
                  UIFile * f = *it;
                  if (!f || (!f->isDirectory() && !f->getSize())) {
                    continue;
                  }
                  unsigned int id = global->getEngine()->newTransferJobDownload(std::static_pointer_cast<BrowseScreenSite>(active)->siteName(),
                                                                                activefl,
                                                                                f->getName(),
                                                                                otherfl->getPath());
                  ui->addTempLegendTransferJob(id);
                }
              }
            }
          }
        }
        else if (other->type() == BrowseScreenType::SITE) {
          std::shared_ptr<LocalFileList> activefl = std::static_pointer_cast<BrowseScreenLocal>(active)->fileList();
          std::shared_ptr<FileList> otherfl = std::static_pointer_cast<BrowseScreenSite>(other)->fileList();
          std::list<UIFile *> files = std::static_pointer_cast<BrowseScreenLocal>(active)->getUIFileList()->getSelectedFiles();
          if (!!activefl && otherfl != NULL) {
            for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
              UIFile * f = *it;
              if (!f || (!f->isDirectory() && !f->getSize())) {
                continue;
              }
              unsigned int id = global->getEngine()->newTransferJobUpload(activefl->getPath(),
                                                                          f->getName(),
                                                                          std::static_pointer_cast<BrowseScreenSite>(other)->siteName(),
                                                                          otherfl);
              ui->addTempLegendTransferJob(id);
            }
          }
        }
      }
      return true;
    case 'u':
      toggleCompareListMode(CompareMode::UNIQUE);
      return true;
    case 'i':
      toggleCompareListMode(CompareMode::IDENTICAL);
      return true;
    case 'm':
      if (active->type() == BrowseScreenType::SITE) {
        std::shared_ptr<BrowseScreenSite> activesite = std::static_pointer_cast<BrowseScreenSite>(active);
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        std::string dstpath;
        if (split && other->type() == BrowseScreenType::SITE) {
          std::shared_ptr<BrowseScreenSite> othersite = std::static_pointer_cast<BrowseScreenSite>(other);
          if (activesite->getSite() == othersite->getSite()) {
            dstpath = othersite->getUIFileList()->getPath().toString();
          }
        }
        activesite->initiateMove(dstpath);
      }
      return true;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      int hotkey = ch - '0';
      Section * section = global->getSectionManager()->getSection(hotkey);
      if (section == nullptr) {
        return true;
      }
      if (left->type() == BrowseScreenType::SITE) {
        std::shared_ptr<Site> site = std::static_pointer_cast<BrowseScreenSite>(left)->getSite();
        if (site->hasSection(section->getName())) {
          Path path = site->getSectionPath(section->getName());
          std::static_pointer_cast<BrowseScreenSite>(left)->gotoPath(path);
        }
      }
      if (split && right->type() == BrowseScreenType::SITE) {
        std::shared_ptr<Site> site = std::static_pointer_cast<BrowseScreenSite>(right)->getSite();
        if (site->hasSection(section->getName())) {
          Path path = site->getSectionPath(section->getName());
          std::static_pointer_cast<BrowseScreenSite>(right)->gotoPath(path);
        }
      }
      return true;
    }
  }
  return false;
}

std::string BrowseScreen::getLegendText() const {
  std::string extra = "";
  if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
    extra += "Show [u]niques - Show [i]denticals - ";
    if (left->type() == BrowseScreenType::SITE || right->type() == BrowseScreenType::SITE) {
      extra += "[t]ransfer - ";
    }
  }
  return "[Tab] switch side - " + extra  + active->getLegendText();
}

std::string BrowseScreen::getInfoLabel() const {
  if (split) {
   if (left->type() == BrowseScreenType::SITE) {
     if (right->type() == BrowseScreenType::SITE) {
       return "BROWSING: " + std::static_pointer_cast<BrowseScreenSite>(left)->siteName() + " - " +
           std::static_pointer_cast<BrowseScreenSite>(right)->siteName();
     }
     return left->getInfoLabel();
   }
   if (right->type() == BrowseScreenType::SITE) {
     return right->getInfoLabel();
   }
  }
  return active->getInfoLabel();
}

std::string BrowseScreen::getInfoText() const {
  return active->getInfoText();
}

bool BrowseScreen::isInitialized() const {
  return !!active;
}

void BrowseScreen::suggestOtherRefresh(BrowseScreenSub* sub) {
  if (split && left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::SITE) {
    if (std::static_pointer_cast<BrowseScreenSite>(left)->getSite() == std::static_pointer_cast<BrowseScreenSite>(right)->getSite()) {
      if (sub == left.get()) {
        right->refreshFileList();
      }
      else {
        left->refreshFileList();
      }
    }
  }
}

void BrowseScreen::switchSide() {
  initsplitupdate = false;
  bool leftfocused = active == left;
  left->setFocus(!leftfocused);
  right->setFocus(leftfocused);
  if (leftfocused) {
    left->update();
    active = right;
  }
  else {
    right->update();
    active = left;
  }
  ui->setInfo();
  if (left->type() != right->type()) {
    ui->setLegend();
  }
  ui->redraw();
}

void BrowseScreen::closeSide() {
  if (split) {
    initsplitupdate = false;
    split = false;
    if (active == left) {
      left = right;
    }
    else {
      right.reset();
    }
    active = left;
    if (active->type() == BrowseScreenType::SELECTOR) {
      ui->returnToLast();
      return;
    }
    active->setFocus(true);
    ui->redraw();
    ui->setInfo();
    ui->setLegend();
  }
  else {
    ui->returnToLast();
  }
}

void BrowseScreen::toggleCompareListMode(CompareMode mode) {
  if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
    UIFileList * leftlist = left->getUIFileList();
    UIFileList * rightlist = right->getUIFileList();
    if (leftlist->getCompareListMode() == mode || rightlist->getCompareListMode() == mode) {
      leftlist->clearCompareListMode();
      rightlist->clearCompareListMode();
    }
    else {
      std::set<std::string> leftfiles;  // need to make copies of the lists here since setting
      std::set<std::string> rightfiles; // it on one side will affect the other side
      const std::vector <UIFile *> * sorteduniquelist = leftlist->getSortedList();
      for (std::vector<UIFile *>::const_iterator it = sorteduniquelist->begin(); it != sorteduniquelist->end(); it++) {
        if (*it != nullptr) {
          leftfiles.insert((*it)->getName());
        }
      }
      sorteduniquelist = rightlist->getSortedList();
      for (std::vector<UIFile *>::const_iterator it = sorteduniquelist->begin(); it != sorteduniquelist->end(); it++) {
        if (*it != nullptr) {
          rightfiles.insert((*it)->getName());
        }
      }
      leftlist->setCompareList(rightfiles, mode);
      rightlist->setCompareList(leftfiles, mode);
    }
  }
  ui->redraw();
  ui->setInfo();
}

void BrowseScreen::clearCompareListMode() {
  if (left && left->type() != BrowseScreenType::SELECTOR) {
    left->getUIFileList()->clearCompareListMode();
  }
  if (right && right->type() != BrowseScreenType::SELECTOR) {
    right->getUIFileList()->clearCompareListMode();
  }
}
