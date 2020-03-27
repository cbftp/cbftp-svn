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

BrowseScreen::BrowseScreen(Ui* ui) : UIWindow(ui, "BrowseScreen"),
  sitekeybinds("BrowseScreenSite"), localkeybinds("BrowseScreenLocal")
{
  keybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('c', KEYACTION_CLOSE, "Close");
  keybinds.addBind(KEY_LEFT, KEYACTION_CLOSE2, "Close");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  keybinds.addBind(10, KEYACTION_ENTER, "Browse");
  keybinds.addBind('b', KEYACTION_BROWSE, "Browse");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Browse");
  keybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  keybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  sitekeybinds.addScope(KEYSCOPE_SPLIT_SITE_SITE, "When split browsing site-site");
  sitekeybinds.addScope(KEYSCOPE_SPLIT_SITE_LOCAL, "When split browsing site-local");
  sitekeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (FXP)", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (Download)", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  sitekeybinds.addBind('c', KEYACTION_CLOSE, "Close");
  sitekeybinds.addBind(10, KEYACTION_ENTER, "Open");
  sitekeybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Open");
  sitekeybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Return");
  sitekeybinds.addBind(KEY_BACKSPACE, KEYACTION_RETURN, "Return");
  sitekeybinds.addBind('r', KEYACTION_SPREAD, "Spread");
  sitekeybinds.addBind('v', KEYACTION_VIEW_FILE, "View file");
  sitekeybinds.addBind('D', KEYACTION_DOWNLOAD, "Download");
  sitekeybinds.addBind('b', KEYACTION_BIND_SECTION, "Bind to section");
  sitekeybinds.addBind('s', KEYACTION_SORT, "Sort");
  sitekeybinds.addBind('S', KEYACTION_SORT_DEFAULT, "Default sort");
  sitekeybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command");
  sitekeybinds.addBind('W', KEYACTION_WIPE, "Wipe");
  sitekeybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete");
  sitekeybinds.addBind('n', KEYACTION_NUKE, "Nuke");
  sitekeybinds.addBind('M', KEYACTION_MKDIR, "Make directory");
  sitekeybinds.addBind('p', KEYACTION_TOGGLE_SEPARATORS, "Toggle separators");
  sitekeybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  sitekeybinds.addBind('f', KEYACTION_FILTER, "Toggle filter");
  sitekeybinds.addBind('F', KEYACTION_FILTER_REGEX, "Regex filter");
  sitekeybinds.addBind('l', KEYACTION_COMMAND_LOG, "View command log");
  sitekeybinds.addBind(337, KEYACTION_SOFT_SELECT_UP, "Soft select up");
  sitekeybinds.addBind(336, KEYACTION_SOFT_SELECT_DOWN, "Soft select down");
  sitekeybinds.addBind(' ', KEYACTION_HARD_SELECT, "Hard select");
  sitekeybinds.addBind('A', KEYACTION_SELECT_ALL, "Select all");
  sitekeybinds.addBind(1, KEYACTION_SELECT_ALL2, "Select all");
  sitekeybinds.addBind('m', KEYACTION_MOVE, "Move/rename");
  sitekeybinds.addBind('i', KEYACTION_INFO, "File info");
  sitekeybinds.addBind(KEY_F(5), KEYACTION_REFRESH, "Refresh dir");
  sitekeybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  sitekeybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  sitekeybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  sitekeybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  sitekeybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  sitekeybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  sitekeybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  sitekeybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  localkeybinds.addScope(KEYSCOPE_SPLIT_SITE_LOCAL, "When split browsing local-site");
  localkeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (Upload)", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  localkeybinds.addBind('c', KEYACTION_CLOSE, "Close");
  localkeybinds.addBind(10, KEYACTION_ENTER, "Open");
  localkeybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Open");
  localkeybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Return");
  localkeybinds.addBind(KEY_BACKSPACE, KEYACTION_RETURN, "Return");
  localkeybinds.addBind('v', KEYACTION_VIEW_FILE, "View file");
  localkeybinds.addBind('s', KEYACTION_SORT, "Sort");
  localkeybinds.addBind('S', KEYACTION_SORT_DEFAULT, "Default sort");
  localkeybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete");
  localkeybinds.addBind('W', KEYACTION_WIPE, "Delete");
  localkeybinds.addBind('p', KEYACTION_TOGGLE_SEPARATORS, "Toggle separators");
  localkeybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  localkeybinds.addBind('f', KEYACTION_FILTER, "Toggle filter");
  localkeybinds.addBind('F', KEYACTION_FILTER_REGEX, "Regex filter");
  localkeybinds.addBind(337, KEYACTION_SOFT_SELECT_UP, "Soft select up");
  localkeybinds.addBind(336, KEYACTION_SOFT_SELECT_DOWN, "Soft select down");
  localkeybinds.addBind(' ', KEYACTION_HARD_SELECT, "Hard select");
  localkeybinds.addBind('A', KEYACTION_SELECT_ALL, "Select all");
  localkeybinds.addBind(1, KEYACTION_SELECT_ALL2, "Select all");
  localkeybinds.addBind(KEY_F(5), KEYACTION_REFRESH, "Refresh dir");
  localkeybinds.addBind('i', KEYACTION_INFO, "File info");
  localkeybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  localkeybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  localkeybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  localkeybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  localkeybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  localkeybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  localkeybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  localkeybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  allowimplicitgokeybinds = false;
}

BrowseScreen::~BrowseScreen() {

}

void BrowseScreen::initialize(unsigned int row, unsigned int col, ViewMode viewmode, const std::string & site, const Path path) {
  expectbackendpush = true;
  this->split = initsplitupdate = viewmode == VIEW_SPLIT;
  TimeReference::updateTime();
  if (viewmode != VIEW_LOCAL) {
    left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site, path);
  }
  else {
    left = std::make_shared<BrowseScreenLocal>(ui, localkeybinds);
  }
  if (split) {
    left->setFocus(false);
    active = right = std::make_shared<BrowseScreenSelector>(ui, keybinds);
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
            active = left = std::make_shared<BrowseScreenSelector>(ui, localkeybinds);
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
            active = right = std::make_shared<BrowseScreenSelector>(ui, localkeybinds);
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
      active = (active == left ? left : right) = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, op.getArg(), targetpath);
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    }
    case BROWSESCREENACTION_HOME:
      active = (active == left ? left : right) = std::make_shared<BrowseScreenLocal>(ui, localkeybinds);
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
  int scope = KEYSCOPE_ALL;
  if (split) {
    if (left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::SITE) {
      scope = KEYSCOPE_SPLIT_SITE_SITE;
    }
    if ((left->type() == BrowseScreenType::LOCAL && right->type() == BrowseScreenType::SITE) ||
        (left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::LOCAL))
    {
      scope = KEYSCOPE_SPLIT_SITE_LOCAL;
    }
  }
  KeyBinds* binds = &keybinds;
  if (active->type() == BrowseScreenType::SITE) {
    binds = &sitekeybinds;
  }
  else if (active->type() == BrowseScreenType::LOCAL) {
    binds = &localkeybinds;
  }
  int action = binds->getKeyAction(ch, scope);
  switch (action) {
    case KEYACTION_SPLIT:
      if (!split) {
        split = true;
        right = std::make_shared<BrowseScreenSelector>(ui, keybinds);
      }
      {
        switchSide();
      }
      return true;
    case KEYACTION_TRANSFER:
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
    case KEYACTION_COMPARE_UNIQUE:
      toggleCompareListMode(CompareMode::UNIQUE);
      return true;
    case KEYACTION_COMPARE_IDENTICAL:
      toggleCompareListMode(CompareMode::IDENTICAL);
      return true;
    case KEYACTION_MOVE:
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
    case KEYACTION_0:
      jumpSectionHotkey(0);
      return true;
    case KEYACTION_1:
      jumpSectionHotkey(1);
      return true;
    case KEYACTION_2:
      jumpSectionHotkey(2);
      return true;
    case KEYACTION_3:
      jumpSectionHotkey(3);
      return true;
    case KEYACTION_4:
      jumpSectionHotkey(4);
      return true;
    case KEYACTION_5:
      jumpSectionHotkey(5);
      return true;
    case KEYACTION_6:
      jumpSectionHotkey(6);
      return true;
    case KEYACTION_7:
      jumpSectionHotkey(7);
      return true;
    case KEYACTION_8:
      jumpSectionHotkey(8);
      return true;
    case KEYACTION_9:
      jumpSectionHotkey(9);
      return true;
    case KEYACTION_KEYBINDS:
      ui->goKeyBinds(binds);
      return true;
  }
  return false;
}

std::string BrowseScreen::getLegendText() const {
  return active->getLegendText(KEYSCOPE_ALL);
  /*std::string extra = "";
  if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
    extra += "Show [u]niques - Show [I]denticals - ";
    if (left->type() == BrowseScreenType::SITE || right->type() == BrowseScreenType::SITE) {
      extra += "[t]ransfer - ";
    }
  }
  return "[Tab] switch side - " + extra  + active->getLegendText();*/
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

void BrowseScreen::jumpSectionHotkey(int hotkey) {
  Section * section = global->getSectionManager()->getSection(hotkey);
  if (section == nullptr) {
    return;
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
}
