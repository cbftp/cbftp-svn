#include "ui.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <cassert>

#include "../core/workmanager.h"
#include "../core/tickpoke.h"
#include "../core/iomanager.h"
#include "../core/signal.h"
#include "../globalcontext.h"
#include "../externalfileviewing.h"
#include "../engine.h"
#include "../datafilehandler.h"
#include "../path.h"

#include "legendprinterkeybinds.h"
#include "legendprinterspreadjob.h"
#include "legendprintertransferjob.h"
#include "legendwindow.h"
#include "infowindow.h"
#include "uiwindow.h"
#include "chardraw.h"
#include "termint.h"

#include "screens/loginscreen.h"
#include "screens/newkeyscreen.h"
#include "screens/mainscreen.h"
#include "screens/editsitescreen.h"
#include "screens/confirmationscreen.h"
#include "screens/sitestatusscreen.h"
#include "screens/rawdatascreen.h"
#include "screens/browsescreen.h"
#include "screens/newracescreen.h"
#include "screens/racestatusscreen.h"
#include "screens/rawcommandscreen.h"
#include "screens/globaloptionsscreen.h"
#include "screens/skiplistscreen.h"
#include "screens/changekeyscreen.h"
#include "screens/eventlogscreen.h"
#include "screens/proxyoptionsscreen.h"
#include "screens/editproxyscreen.h"
#include "screens/viewfilescreen.h"
#include "screens/nukescreen.h"
#include "screens/fileviewersettingsscreen.h"
#include "screens/scoreboardscreen.h"
#include "screens/selectsitesscreen.h"
#include "screens/transfersscreen.h"
#include "screens/transferjobstatusscreen.h"
#include "screens/allracesscreen.h"
#include "screens/alltransferjobsscreen.h"
#include "screens/transferstatusscreen.h"
#include "screens/transfersfilterscreen.h"
#include "screens/infoscreen.h"
#include "screens/selectjobsscreen.h"
#include "screens/makedirscreen.h"
#include "screens/sectionsscreen.h"
#include "screens/editsectionscreen.h"
#include "screens/sitesectionsscreen.h"
#include "screens/editsitesectionscreen.h"
#include "screens/siteslotsscreen.h"
#include "screens/snakescreen.h"
#include "screens/disableencryptionscreen.h"
#include "screens/movescreen.h"

namespace {

static Ui * instance = new Ui();

} // namespace

Ui::Ui() :
  main(nullptr),
  ticker(0),
  haspushed(false),
  pushused(false),
  legendenabled(false),
  infoenabled(false),
  dead(false),
  legendmode(LEGEND_SCROLLING),
  split(false),
  fullscreentoggle(false)
{
}

Ui::~Ui() {

}

bool Ui::init() {

  Core::registerSignalDispatch(SIGWINCH, global->getWorkManager(), this);

  CharDraw::init();

  global->getSettingsLoaderSaver()->addSettingsAdder(this);

  thread.start("cbftp-ui", this);

  initret = true;
  uiqueue.push(UICommand(UI_COMMAND_INIT));
  eventcomplete.wait();
  if (!initret) {
    return false;
  }
  legendwindow = std::make_shared<LegendWindow>(this, legend, 2, col);
  infowindow = std::make_shared<InfoWindow>(this, info, 2, col);
  loginscreen = std::make_shared<LoginScreen>(this);
  newkeyscreen = std::make_shared<NewKeyScreen>(this);
  mainscreen = std::make_shared<MainScreen>(this);
  confirmationscreen = std::make_shared<ConfirmationScreen>(this);
  editsitescreen = std::make_shared<EditSiteScreen>(this);
  sitestatusscreen = std::make_shared<SiteStatusScreen>(this);
  rawdatascreen = std::make_shared<RawDataScreen>(this);
  rawcommandscreen = std::make_shared<RawCommandScreen>(this);
  browsescreen = std::make_shared<BrowseScreen>(this);
  newracescreen = std::make_shared<NewRaceScreen>(this);
  racestatusscreen = std::make_shared<RaceStatusScreen>(this);
  globaloptionsscreen = std::make_shared<GlobalOptionsScreen>(this);
  skiplistscreen = std::make_shared<SkipListScreen>(this);
  changekeyscreen = std::make_shared<ChangeKeyScreen>(this);
  eventlogscreen = std::make_shared<EventLogScreen>(this);
  proxyoptionsscreen = std::make_shared<ProxyOptionsScreen>(this);
  editproxyscreen = std::make_shared<EditProxyScreen>(this);
  viewfilescreen = std::make_shared<ViewFileScreen>(this);
  nukescreen = std::make_shared<NukeScreen>(this);
  fileviewersettingsscreen = std::make_shared<FileViewerSettingsScreen>(this);
  scoreboardscreen = std::make_shared<ScoreBoardScreen>(this);
  selectsitesscreen = std::make_shared<SelectSitesScreen>(this);
  transfersscreen = std::make_shared<TransfersScreen>(this);
  transferjobstatusscreen = std::make_shared<TransferJobStatusScreen>(this);
  allracesscreen = std::make_shared<AllRacesScreen>(this);
  alltransferjobsscreen = std::make_shared<AllTransferJobsScreen>(this);
  transferstatusscreen = std::make_shared<TransferStatusScreen>(this);
  transfersfilterscreen = std::make_shared<TransfersFilterScreen>(this);
  infoscreen = std::make_shared<InfoScreen>(this);
  selectjobsscreen = std::make_shared<SelectJobsScreen>(this);
  makedirscreen = std::make_shared<MakeDirScreen>(this);
  sectionsscreen = std::make_shared<SectionsScreen>(this);
  editsectionscreen = std::make_shared<EditSectionScreen>(this);
  sitesectionsscreen = std::make_shared<SiteSectionsScreen>(this);
  editsitesectionscreen = std::make_shared<EditSiteSectionScreen>(this);
  siteslotsscreen = std::make_shared<SiteSlotsScreen>(this);
  snakescreen = std::make_shared<SnakeScreen>(this);
  disableencryptionscreen = std::make_shared<DisableEncryptionScreen>(this);
  movescreen = std::make_shared<MoveScreen>(this);
  mainwindows.push_back(mainscreen);
  mainwindows.push_back(newkeyscreen);
  mainwindows.push_back(confirmationscreen);
  mainwindows.push_back(editsitescreen);
  mainwindows.push_back(sitestatusscreen);
  mainwindows.push_back(rawdatascreen);
  mainwindows.push_back(rawcommandscreen);
  mainwindows.push_back(browsescreen);
  mainwindows.push_back(newracescreen);
  mainwindows.push_back(racestatusscreen);
  mainwindows.push_back(globaloptionsscreen);
  mainwindows.push_back(skiplistscreen);
  mainwindows.push_back(changekeyscreen);
  mainwindows.push_back(eventlogscreen);
  mainwindows.push_back(proxyoptionsscreen);
  mainwindows.push_back(editproxyscreen);
  mainwindows.push_back(viewfilescreen);
  mainwindows.push_back(nukescreen);
  mainwindows.push_back(fileviewersettingsscreen);
  mainwindows.push_back(scoreboardscreen);
  mainwindows.push_back(selectsitesscreen);
  mainwindows.push_back(transfersscreen);
  mainwindows.push_back(transferjobstatusscreen);
  mainwindows.push_back(allracesscreen);
  mainwindows.push_back(alltransferjobsscreen);
  mainwindows.push_back(transferstatusscreen);
  mainwindows.push_back(transfersfilterscreen);
  mainwindows.push_back(infoscreen);
  mainwindows.push_back(selectjobsscreen);
  mainwindows.push_back(makedirscreen);
  mainwindows.push_back(sectionsscreen);
  mainwindows.push_back(editsectionscreen);
  mainwindows.push_back(sitesectionsscreen);
  mainwindows.push_back(editsitesectionscreen);
  mainwindows.push_back(siteslotsscreen);
  mainwindows.push_back(snakescreen);
  mainwindows.push_back(disableencryptionscreen);
  mainwindows.push_back(movescreen);

  legendprinterkeybinds = std::make_shared<LegendPrinterKeybinds>(this);
  legendwindow->setMainLegendPrinter(legendprinterkeybinds);

  if (global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_ENCRYPTED) {
    loginscreen->initialize(mainrow, maincol);
    topwindow = loginscreen;
  }
  else {
    mainscreen->initialize(mainrow, maincol);
    topwindow = mainscreen;
    enableInfo();
    enableLegend();
  }
  infowindow->setLabel(topwindow->getInfoLabel());
  std::cin.putback('#'); // needed to be able to peek properly
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
  global->getIOManager()->registerExternalFD(this, STDIN_FILENO);
  global->getTickPoke()->startPoke(this, "UI", 50, 0);
  return true;
}

void Ui::initIntern() {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  curs_set(0);
  refresh();
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    endwin();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    initret = false;
    eventcomplete.post();
    return;
  }
#if NCURSES_EXT_FUNCS >= 20081102
  set_escdelay(25);
#else
  ESCDELAY = 25;
#endif
  mainrow = row;
  maincol = col;
  main = newwin(row, col, 0, 0);
  legend = newwin(2, col, row - 2, 0);
  info = newwin(2, col, 0, 0);
  keypad(stdscr, TRUE);
  noecho();
  eventcomplete.post();
}

void Ui::backendPush() {
  haspushed = true;
  if (!pushused) {
    topwindow->update();
    uiqueue.push(UICommand(UI_COMMAND_REFRESH));
    pushused = true;
  }
}

void Ui::signal(int signal, int) {
  if (signal == SIGWINCH) {
    terminalSizeChanged();
  }
  else {
    assert(false);
  }
}

void Ui::terminalSizeChanged() {
  uiqueue.push(UICommand(UI_COMMAND_RESIZE));
  eventcomplete.wait();
  redrawAll();
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

LegendMode Ui::legendMode() const {
  return legendmode;
}

void Ui::setLegendMode(LegendMode newmode) {
  LegendMode oldmode = legendmode;
  legendmode = newmode;
  if (newmode != oldmode) {
    if (oldmode == LEGEND_DISABLED) {
      enableLegend();
    }
    else if (newmode == LEGEND_DISABLED) {
      disableLegend();
    }
    else {
      redrawAll();
    }
  }
}

void Ui::kill() {
  uiqueue.push(UICommand(UI_COMMAND_KILL));
  for (int i = 0; i < 10; i++) {
    usleep(100000);
    if (dead) {
      break;
    }
  }
  if (!dead) {
    endwin();
  }
}

void Ui::refreshAll() {
  if (legendenabled) {
    wnoutrefresh(legend);
  }
  if (infoenabled) {
    wnoutrefresh(info);
  }
  wnoutrefresh(main);
  doupdate();
}

void Ui::FDData(int sockid) {
  int ch = getch();
  if (!topwindow->keyPressed(ch)) {
    globalKeyBinds(ch);
  }
}

void Ui::enableInfo() {
  if (!infoenabled) {
    infoenabled = true;
    mainrow = mainrow - 2;
    redrawAll();
    uiqueue.push(UICommand(UI_COMMAND_ADJUST_INFO, true));
  }
}

void Ui::disableInfo() {
  if (infoenabled) {
    infoenabled = false;
    mainrow = mainrow + 2;
    redrawAll();
    uiqueue.push(UICommand(UI_COMMAND_ADJUST_INFO, false));
  }
}

void Ui::enableLegend() {
  if (!legendenabled && legendMode() != LEGEND_DISABLED) {
    legendenabled = true;
    mainrow = mainrow - 2;
    redrawAll();
    uiqueue.push(UICommand(UI_COMMAND_ADJUST_LEGEND));
  }
}

void Ui::disableLegend() {
  if (legendenabled) {
    legendenabled = false;
    mainrow = mainrow + 2;
    redrawAll();
    uiqueue.push(UICommand(UI_COMMAND_ADJUST_LEGEND));
  }
}

void Ui::redrawAll() {
  std::vector<std::shared_ptr<UIWindow> >::iterator it;
  for (it = mainwindows.begin(); it != mainwindows.end(); it++) {
    (*it)->resize(mainrow, maincol);
  }
  if (legendMode() != LEGEND_DISABLED) {
    legendwindow->resize(2, col);
    legendwindow->redraw();
  }
  if (infoenabled) {
    infowindow->resize(2, col);
    infowindow->redraw();
  }
  if (!!topwindow) {
    topwindow->resize(mainrow, maincol);
    topwindow->redraw();
  }
}

void Ui::tick(int message) {
  if (!(ticker++ % 5)) {
    bool refresh = false;
    if (topwindow->autoUpdate()) {
      topwindow->update();
      refresh = true;
    }
    if (legendenabled) {
      refresh = true;
      legendwindow->update();
    }
    if (infoenabled) {
      refresh = true;
      infowindow->setText(topwindow->getInfoText());
    }
    if (refresh) {
      uiqueue.push(UICommand(UI_COMMAND_REFRESH));
    }
  }
  pushused = false;
  if (haspushed) {
    backendPush();
    haspushed = false;
  }
}

void Ui::run() {
  while(1) {
    UICommand command = uiqueue.pop();
    switch (command.getCommand()) {
      case UI_COMMAND_INIT:
        initIntern();
        break;
      case UI_COMMAND_REFRESH:
        refreshAll();
        break;
      case UI_COMMAND_HIGHLIGHT_OFF:
        wattroff(command.getWindow(), A_REVERSE);
        break;
      case UI_COMMAND_HIGHLIGHT_ON:
        wattron(command.getWindow(), A_REVERSE);
        break;
      case UI_COMMAND_CURSOR_SHOW:
        curs_set(1);
        break;
      case UI_COMMAND_CURSOR_HIDE:
        curs_set(0);
        break;
      case UI_COMMAND_CURSOR_MOVE:
        TermInt::moveCursor(command.getWindow(), command.getRow(), command.getCol());
        break;
      case UI_COMMAND_ERASE:
        werase(command.getWindow());
        break;
      case UI_COMMAND_PRINT_STR:
        TermInt::printStr(command.getWindow(), command.getRow(), command.getCol(),
            command.getText(), command.getMaxlen(), command.getRightAlign());
        break;
      case UI_COMMAND_PRINT_WIDE_STR:
        TermInt::printStr(command.getWindow(), command.getRow(), command.getCol(),
            command.getWideText(), command.getMaxlen(), command.getRightAlign());
        break;
      case UI_COMMAND_PRINT_CHAR:
        TermInt::printChar(command.getWindow(), command.getRow(), command.getCol(),
            command.getChar());
        break;
      case UI_COMMAND_RESIZE:
        resizeTerm();
        break;
      case UI_COMMAND_ADJUST_LEGEND:
        wresize(main, mainrow, maincol);
        refreshAll();
        break;
      case UI_COMMAND_ADJUST_INFO:
        wresize(main, mainrow, maincol);
        if (command.getShow()) {
          mvwin(main, 2, 0);
        }
        else {
          mvwin(main, 0, 0);
        }
        refreshAll();
        break;
      case UI_COMMAND_KILL:
        endwin();
        dead = true;
        return;
      case UI_COMMAND_BELL:
        beep();
        break;
    }
  }
}

void Ui::switchToWindow(std::shared_ptr<UIWindow> window, bool allowsplit, bool doredraw) {
  history.push_back(topwindow);
  if (split && !allowsplit) {
    setSplit(false);
  }
  setLegendText(window->getLegendText());
  infowindow->setLabel(window->getInfoLabel());
  infowindow->setText(window->getInfoText());
  topwindow = window;
  if (doredraw) {
    topwindow->redraw();
  }
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::globalKeyBinds(int ch) {
  bool match = true;
  switch(ch) {
    case 'K':
      global->getExternalFileViewing()->killAll();
      break;
    case 'p':
      global->getEngine()->startLatestPreparedRace();
      break;
    case 'N':
      global->getEngine()->toggleStartNextPreparedRace();
      break;
    case '\\':
      if (fullscreentoggle) {
        enableInfo();
        enableLegend();
        fullscreentoggle = false;
      }
      else {
        disableInfo();
        disableLegend();
        fullscreentoggle = true;
      }
      break;
    default:
      match = false;
      break;
  }
  if (match) {
    update();
  }
}

void Ui::resizeTerm() {
  struct winsize size;
  bool trytodraw = false;
  if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
    if (size.ws_row >= 5 && size.ws_col >= 10) {
      trytodraw = true;
    }
  }
  if (trytodraw) {
    resizeterm(size.ws_row, size.ws_col);
    endwin();
    timeout(0);
    while (getch() != ERR);
    timeout(-1);
    refresh();
    getmaxyx(stdscr, row, col);
    unsigned int newmainrow = row;
    if (legendenabled) {
      newmainrow = newmainrow - 2;
    }
    if (infoenabled) {
      newmainrow = newmainrow - 2;
    }
    maincol = col;
    mainrow = newmainrow;
    wresize(main, mainrow, maincol);
    wresize(legend, 2, col);
    wresize(info, 2, col);
    mvwin(legend, row - 2, 0);
  }
  eventcomplete.post();
}

void Ui::returnToLast() {
  switchToLast();
  topwindow->redraw();
  infowindow->setText(topwindow->getInfoText());
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::update() {
  topwindow->update();
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::setLegend() {
  setLegendText(topwindow->getLegendText());
  infowindow->setText(topwindow->getInfoText());
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::addTempLegendTransferJob(unsigned int id) {
  std::shared_ptr<LegendPrinterTransferJob> lptj = std::make_shared<LegendPrinterTransferJob>(this, id);
  legendwindow->addTempLegendPrinter(lptj);
  legendwindow->redraw();
}

void Ui::addTempLegendSpreadJob(unsigned int id) {
  std::shared_ptr<LegendPrinterSpreadJob> lpsj = std::make_shared<LegendPrinterSpreadJob>(this, id);
  legendwindow->addTempLegendPrinter(lpsj);
  legendwindow->redraw();
}

void Ui::setInfo() {
  infowindow->setLabel(topwindow->getInfoLabel());
  infowindow->setText(topwindow->getInfoText());
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::setSplit(bool split) {
  if (this->split != split) {
    this->split = split;
    infowindow->setSplit(split);
    infowindow->redraw();
    legendwindow->setSplit(split);
    legendwindow->redraw();
  }
}

void Ui::redraw() {
  topwindow->redraw();
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::hideCursor() {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_HIDE));
}

void Ui::showCursor() {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_SHOW));
}

void Ui::erase() {
  erase(main);
}

void Ui::erase(WINDOW * window) {
  uiqueue.push(UICommand(UI_COMMAND_ERASE, window));
}

void Ui::moveCursor(unsigned int row, unsigned int col) {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_MOVE, main, row, col));
}

void Ui::highlight(bool highlight, WINDOW * window) {
  if (highlight) {
    uiqueue.push(UICommand(UI_COMMAND_HIGHLIGHT_ON, window));
  }
  else {
    uiqueue.push(UICommand(UI_COMMAND_HIGHLIGHT_OFF, window));
  }
}

void Ui::printStr(unsigned int row, unsigned int col, const std::string & str, bool highlight, int maxlen, bool rightalign, WINDOW * window) {
  if (!window) {
    window = main;
  }
  if (highlight) {
    this->highlight(true, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_STR, window, row, col, str, maxlen, rightalign));
  if (highlight) {
    this->highlight(false, window);
  }
}



void Ui::printStr(unsigned int row, unsigned int col, const std::basic_string<unsigned int> & str, bool highlight, int maxlen, bool rightalign, WINDOW * window) {
  if (!window) {
    window = main;
  }
  if (highlight) {
    this->highlight(true, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_WIDE_STR, window, row, col, str, maxlen, rightalign));
  if (highlight) {
    this->highlight(false, window);
  }
}

void Ui::printChar(unsigned int row, unsigned int col, unsigned int c, bool highlight, WINDOW * window) {
  if (!window) {
    window = main;
  }
  if (highlight) {
    this->highlight(true, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_CHAR, window, row, col, c));
  if (highlight) {
    this->highlight(false, window);
  }
}

void Ui::goRawCommand(const std::string & site) {
  rawcommandscreen->initialize(mainrow, maincol, site);
  switchToWindow(rawcommandscreen);
}

void Ui::goRawCommand(const std::string & site, const Path & path, const std::string & arg) {
  rawcommandscreen->initialize(mainrow, maincol, site, path, arg);
  switchToWindow(rawcommandscreen);
}

void Ui::goInfo(const std::string & message) {
  confirmationscreen->initialize(mainrow, maincol, message, ConfirmationMode::INFO);
  switchToWindow(confirmationscreen);
}

void Ui::goConfirmation(const std::string & message) {
  confirmationscreen->initialize(mainrow, maincol, message, ConfirmationMode::NORMAL);
  switchToWindow(confirmationscreen);
}

void Ui::goStrongConfirmation(const std::string & message) {
  confirmationscreen->initialize(mainrow, maincol, message, ConfirmationMode::STRONG);
  switchToWindow(confirmationscreen);
}

void Ui::goNuke(const std::string & site, const std::string & items, const Path & path) {
  nukescreen->initialize(mainrow, maincol, site, items, path);
  switchToWindow(nukescreen);
}

void Ui::goViewFile(const std::string & site, const std::string & file, const std::shared_ptr<FileList>& fl) {
  viewfilescreen->initialize(mainrow, maincol, site, file, fl);
  switchToWindow(viewfilescreen);
}

void Ui::goViewFile(const Path & dir, const std::string & file) {
  viewfilescreen->initialize(mainrow, maincol, dir, file);
  switchToWindow(viewfilescreen);
}

void Ui::goNewRace(const std::string & site, const std::list<std::string> & sections, const std::list<std::pair<std::string, bool> > & items) {
  newracescreen->initialize(mainrow, maincol, site, sections, items);
  switchToWindow(newracescreen);
}

void Ui::goSelectSites(const std::string & message, std::list<std::shared_ptr<Site> > currentsitelist, std::list<std::shared_ptr<Site> > excludedsitelist) {
  selectsitesscreen->initializeExclude(mainrow, maincol, message, currentsitelist, excludedsitelist);
  switchToWindow(selectsitesscreen);
}

void Ui::goSelectSitesFrom(const std::string & message, std::list<std::shared_ptr<Site> > currentsitelist, std::list<std::shared_ptr<Site> > sitelist) {
  selectsitesscreen->initializeSelect(mainrow, maincol, message, currentsitelist, sitelist);
  switchToWindow(selectsitesscreen);
}

void Ui::goSelectSpreadJobs() {
  selectjobsscreen->initialize(mainrow, maincol, JOBTYPE_SPREADJOB);
  switchToWindow(selectjobsscreen);
}

void Ui::goSelectTransferJobs() {
  selectjobsscreen->initialize(mainrow, maincol, JOBTYPE_TRANSFERJOB);
  switchToWindow(selectjobsscreen);
}

void Ui::goSkiplist(SkipList * skiplist) {
  skiplistscreen->initialize(mainrow, maincol, skiplist);
  switchToWindow(skiplistscreen);
}

void Ui::goChangeKey() {
  changekeyscreen->initialize(mainrow, maincol);
  switchToWindow(changekeyscreen);
}

void Ui::goProxy() {
  proxyoptionsscreen->initialize(mainrow, maincol);
  switchToWindow(proxyoptionsscreen);
}

void Ui::goFileViewerSettings() {
  fileviewersettingsscreen->initialize(mainrow, maincol);
  switchToWindow(fileviewersettingsscreen);
}

void Ui::goSiteStatus(const std::string & site) {
  sitestatusscreen->initialize(mainrow, maincol, site);
  switchToWindow(sitestatusscreen);
}

void Ui::goRaceStatus(unsigned int id) {
  racestatusscreen->initialize(mainrow, maincol, id);
  switchToWindow(racestatusscreen);
}

void Ui::goTransferJobStatus(unsigned int id) {
  transferjobstatusscreen->initialize(mainrow, maincol, id);
  switchToWindow(transferjobstatusscreen);
}

void Ui::goTransferStatus(std::shared_ptr<TransferStatus> ts) {
  transferstatusscreen->initialize(mainrow, maincol, ts);
  switchToWindow(transferstatusscreen);
}

void Ui::goGlobalOptions() {
  globaloptionsscreen->initialize(mainrow, maincol);
  switchToWindow(globaloptionsscreen);
}

void Ui::goEventLog() {
  eventlogscreen->initialize(mainrow, maincol);
  switchToWindow(eventlogscreen);
}

void Ui::goScoreBoard() {
  scoreboardscreen->initialize(mainrow, maincol);
  switchToWindow(scoreboardscreen);
}

void Ui::goTransfers() {
  transfersscreen->initialize(mainrow, maincol);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSite(const std::string & site) {
  transfersscreen->initializeFilterSite(mainrow, maincol, site);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSpreadJob(const std::string & job) {
  transfersscreen->initializeFilterSpreadJob(mainrow, maincol, job);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterTransferJob(const std::string & job) {
  transfersscreen->initializeFilterTransferJob(mainrow, maincol, job);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSpreadJobSite(const std::string & job, const std::string & site) {
  transfersscreen->initializeFilterSpreadJobSite(mainrow, maincol, job, site);
  switchToWindow(transfersscreen);
}

void Ui::returnTransferFilters(const TransferFilteringParameters & tfp) {
  transfersscreen->initialize(mainrow, maincol, tfp);
  switchToLast();
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::goTransfersFiltering(const TransferFilteringParameters & tfp) {
  transfersfilterscreen->initialize(mainrow, maincol, tfp);
  switchToWindow(transfersfilterscreen);
}

void Ui::goEditSite(const std::string & site) {
  editsitescreen->initialize(mainrow, maincol, "edit", site);
  switchToWindow(editsitescreen);
}

void Ui::goAddSite() {
  editsitescreen->initialize(mainrow, maincol, "add", "");
  switchToWindow(editsitescreen);
}

void Ui::goBrowse(const std::string & site, const Path path) {
  browsescreen->initialize(mainrow, maincol, VIEW_NORMAL, site, path);
  switchToWindow(browsescreen);
}

void Ui::goBrowseSplit(const std::string & site) {
  browsescreen->initialize(mainrow, maincol, VIEW_SPLIT, site);
  switchToWindow(browsescreen, true);
}

void Ui::goBrowseLocal() {
  browsescreen->initialize(mainrow, maincol, VIEW_LOCAL);
  switchToWindow(browsescreen);
}

void Ui::goContinueBrowsing() {
  if (browsescreen->isInitialized()) {
    switchToWindow(browsescreen, true, true);
  }
}

void Ui::goAddProxy() {
  editproxyscreen->initialize(mainrow, maincol, "add", "");
  switchToWindow(editproxyscreen);
}

void Ui::goEditProxy(const std::string & proxy) {
  editproxyscreen->initialize(mainrow, maincol, "edit", proxy);
  switchToWindow(editproxyscreen);
}

void Ui::goRawData(const std::string & site) {
  rawdatascreen->initialize(mainrow, maincol, site, 0);
  switchToWindow(rawdatascreen);
}

void Ui::goRawDataJump(const std::string & site, int id) {
  rawdatascreen->initialize(mainrow, maincol, site, id);
  topwindow = rawdatascreen;
  infowindow->setLabel(topwindow->getInfoLabel());
  infowindow->setText(topwindow->getInfoText());
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::goRawBuffer(RawBuffer * rawbuf, const std::string & label, const std::string & infotext) {
  rawcommandscreen->initialize(mainrow, maincol, rawbuf, label, infotext);
  switchToWindow(rawcommandscreen);
}

void Ui::goAllRaces() {
  allracesscreen->initialize(mainrow, maincol);
  switchToWindow(allracesscreen);
}

void Ui::goAllTransferJobs() {
  alltransferjobsscreen->initialize(mainrow, maincol);
  switchToWindow(alltransferjobsscreen);
}

void Ui::goInfo() {
  infoscreen->initialize(mainrow, maincol);
  switchToWindow(infoscreen);
}

void Ui::goMakeDir(const std::string & site, UIFileList & filelist) {
  makedirscreen->initialize(mainrow, maincol, site, filelist);
  switchToWindow(makedirscreen);
}

void Ui::goSiteSlots(const std::shared_ptr<Site> & site) {
  siteslotsscreen->initialize(mainrow, maincol, site);
  switchToWindow(siteslotsscreen);
}

void Ui::goSections() {
  sectionsscreen->initialize(mainrow, maincol);
  switchToWindow(sectionsscreen);
}

void Ui::goSelectSection(const std::list<std::string>& preselected) {
  sectionsscreen->initialize(mainrow, maincol, true, preselected);
  switchToWindow(sectionsscreen);
}

void Ui::goSiteSections(const std::shared_ptr<Site> & site) {
  sitesectionsscreen->initialize(mainrow, maincol, site);
  switchToWindow(sitesectionsscreen);
}

void Ui::goEditSection(const std::string & name) {
  editsectionscreen->initialize(mainrow, maincol, name);
  switchToWindow(editsectionscreen);
}

void Ui::goAddSection() {
  editsectionscreen->initialize(mainrow, maincol);
  switchToWindow(editsectionscreen);
}

void Ui::goAddSiteSection(const std::shared_ptr<Site> & site, const Path & path) {
  editsitesectionscreen->initialize(mainrow, maincol, site, path);
  switchToWindow(editsitesectionscreen);
}

void Ui::goEditSiteSection(const std::shared_ptr<Site> & site, const std::string & section) {
  editsitesectionscreen->initialize(mainrow, maincol, site, section);
  switchToWindow(editsitesectionscreen);
}

void Ui::goSnake() {
  snakescreen->initialize(mainrow, maincol);
  switchToWindow(snakescreen);
}

void Ui::goEnableEncryption() {
  newkeyscreen->initialize(mainrow, maincol);
  switchToWindow(newkeyscreen);
}

void Ui::goDisableEncryption() {
  disableencryptionscreen->initialize(mainrow, maincol);
  switchToWindow(disableencryptionscreen);
}

void Ui::goMove(const std::string& site, const std::string& items, const Path& srcpath, const std::string& dstpath) {
  movescreen->initialize(mainrow, maincol, site, items, srcpath, dstpath);
  switchToWindow(movescreen);
}

void Ui::returnSelectItems(const std::string & items) {
  switchToLast();
  topwindow->command("returnselectitems", items);
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::key(const std::string & key) {
  bool result = global->getSettingsLoaderSaver()->tryDecrypt(key);
  if (result) {
    mainscreen->initialize(mainrow, maincol);
    switchToWindow(mainscreen);
    enableInfo();
    enableLegend();
  }
  else {
    topwindow->update();
    uiqueue.push(UICommand(UI_COMMAND_REFRESH));
  }
}

void Ui::newKey(const std::string & newkey) {
  assert(global->getSettingsLoaderSaver()->setEncrypted(newkey));
  returnToLast();
}

void Ui::confirmYes() {
  switchToLast();
  topwindow->command("yes");
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::confirmNo() {
  switchToLast();
  topwindow->command("no");
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::returnNuke(int multiplier, const std::string & reason) {
  switchToLast();
  topwindow->command("nuke", std::to_string(multiplier) + ";" + reason);
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::returnRaceStatus(unsigned int id) {
  history.clear();
  topwindow = mainscreen;
  racestatusscreen->initialize(mainrow, maincol, id);
  switchToWindow(racestatusscreen);
}

void Ui::returnMakeDir(const std::string & dirname) {
  switchToLast();
  topwindow->command("makedir", dirname);
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::returnMove(const std::string& dstpath) {
  switchToLast();
  topwindow->command("move", dstpath);
  uiqueue.push(UICommand(UI_COMMAND_REFRESH));
}

void Ui::setLegendText(const std::string & legendtext) {
  legendprinterkeybinds->setText(legendtext);
  legendwindow->clearTempLegendPrinters();
  legendwindow->update();
}
void Ui::switchToLast() {
  if (split) {
    setSplit(false);
  }
  topwindow = history.back();
  history.pop_back();
  setLegendText(topwindow->getLegendText());
  infowindow->setLabel(topwindow->getInfoLabel());
  infowindow->setText(topwindow->getInfoText());
}

void Ui::loadSettings(std::shared_ptr<DataFileHandler> dfh) {
  std::vector<std::string> lines;
  dfh->getDataFor("UI", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("legend")) {
      if (!value.compare("true")) {
        setLegendMode(LEGEND_SCROLLING);
      }
      else {
        setLegendMode(LEGEND_DISABLED);
      }
    }
    else if (!setting.compare("legendmode")) {
      setLegendMode((LegendMode) std::stoi(value));
    }
  }
}

void Ui::saveSettings(std::shared_ptr<DataFileHandler> dfh) {
  dfh->addOutputLine("UI", "legendmode=" + std::to_string(legendMode()));
}

void Ui::notify() {
  uiqueue.push(UICommand(UI_COMMAND_BELL));
}

WINDOW * Ui::getLegendWindow() const {
  return legend;
}

void Ui::requestReady(void* service, int requestid) {
  backendPush();
}
