#include "browsescreen.h"

#include <cctype>
#include <algorithm>

#include "../../core/pointer.h"
#include "../../globalcontext.h"
#include "../../util.h"
#include "../../engine.h"
#include "../../localfilelist.h"
#include "../../timereference.h"

#include "../ui.h"
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

void BrowseScreen::initialize(unsigned int row, unsigned int col, ViewMode viewmode, const std::string & sitestr) {
  expectbackendpush = true;
  this->split = initsplitupdate = viewmode == VIEW_SPLIT;
  TimeReference::updateTime();
  if (viewmode != VIEW_LOCAL) {
    left = makePointer<BrowseScreenSite>(ui, sitestr);
  }
  else {
    left = makePointer<BrowseScreenLocal>(ui);
  }
  if (split) {
    left->setFocus(false);
    active = right = makePointer<BrowseScreenSelector>(ui);
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
      if (active->type() != BROWSESCREEN_SELECTOR && split) {
        if (active == left) {
          if (right->type() == BROWSESCREEN_SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = left = makePointer<BrowseScreenSelector>(ui);
            ui->redraw();
            ui->setInfo();
            ui->setLegend();
            return true;
          }
        }
        else {
          if (left->type() == BROWSESCREEN_SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = right = makePointer<BrowseScreenSelector>(ui);
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
    case BROWSESCREENACTION_SITE:
      active = (active == left ? left : right) = makePointer<BrowseScreenSite>(ui, op.getArg());
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    case BROWSESCREENACTION_HOME:
      active = (active == left ? left : right) = makePointer<BrowseScreenLocal>(ui);
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    case BROWSESCREENACTION_NOOP:
      return keyPressedNoSubAction(ch);
    case BROWSESCREENACTION_CAUGHT:
      return true;
  }
  return false;
}

bool BrowseScreen::keyPressedNoSubAction(unsigned int ch) {
  switch (ch) {
    case '\t':
      if (!split) {
        split = true;
        right = makePointer<BrowseScreenSelector>(ui);
      }
      {
        switchSide();
      }
      return true;
    case 't':
      if (split && left->type() != BROWSESCREEN_SELECTOR && right->type() != BROWSESCREEN_SELECTOR) {
        Pointer<BrowseScreenSub> other = active == left ? right : left;
        if (active->type() == BROWSESCREEN_SITE) {
          FileList * activefl = active.get<BrowseScreenSite>()->fileList();
          UIFile * f = active.get<BrowseScreenSite>()->selectedFile();
          if (activefl != NULL && f != NULL && (f->isDirectory() || f->getSize() > 0)) {
            if (other->type() == BROWSESCREEN_SITE) {
              FileList * otherfl = other.get<BrowseScreenSite>()->fileList();
              if (otherfl != NULL) {
                global->getEngine()->newTransferJobFXP(active.get<BrowseScreenSite>()->siteName(),
                                                       activefl,
                                                       other.get<BrowseScreenSite>()->siteName(),
                                                       otherfl,
                                                       f->getName());
              }
            }
            else {
              Pointer<LocalFileList> otherfl = other.get<BrowseScreenLocal>()->fileList();
              if (!!otherfl) {
                global->getEngine()->newTransferJobDownload(active.get<BrowseScreenSite>()->siteName(),
                                                            f->getName(),
                                                            activefl,
                                                            otherfl->getPath());
              }
            }
          }
        }
        else if (other->type() == BROWSESCREEN_SITE) {
          Pointer<LocalFileList> activefl = active.get<BrowseScreenLocal>()->fileList();
          FileList * otherfl = other.get<BrowseScreenSite>()->fileList();
          UIFile * f = active.get<BrowseScreenLocal>()->selectedFile();
          if (!!activefl && otherfl != NULL && f != NULL &&
              (f->isDirectory() || f->getSize() > 0)) {
            global->getEngine()->newTransferJobUpload(activefl->getPath(),
                                                      other.get<BrowseScreenSite>()->siteName(),
                                                      f->getName(),
                                                      otherfl);
          }
        }
      }
      return true;
  }
  return false;
}

std::string BrowseScreen::getLegendText() const {
  std::string transfer = "";
  if (split && left->type() != BROWSESCREEN_SELECTOR && right->type() != BROWSESCREEN_SELECTOR &&
      (left->type() == BROWSESCREEN_SITE || right->type() == BROWSESCREEN_SITE)) {
    transfer = "[t]ransfer - ";
  }
  return "[Tab] switch side - " + transfer  + active->getLegendText();
}

std::string BrowseScreen::getInfoLabel() const {
  if (split) {
   if (left->type() == BROWSESCREEN_SITE) {
     if (right->type() == BROWSESCREEN_SITE) {
       return "BROWSING: " + left.get<BrowseScreenSite>()->siteName() + " - " +
           right.get<BrowseScreenSite>()->siteName();
     }
     return left->getInfoLabel();
   }
   if (right->type() == BROWSESCREEN_SITE) {
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
    if (active->type() == BROWSESCREEN_SELECTOR) {
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
