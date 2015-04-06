#pragma once

#include <string>
#include <list>
#include <vector>
#include <ncurses.h>
#include <pthread.h>

#include "../semaphore.h"
#include "../eventreceiver.h"
#include "uicommand.h"
#include "../blockingqueue.h"
#include "../uibase.h"

#define TICKLENGTH 250000

class UIWindow;
class InfoWindow;
class LegendWindow;
class FileList;
class Site;
class LoginScreen;
class NewKeyScreen;
class MainScreen;
class ConfirmationScreen;
class EditSiteScreen;
class SiteStatusScreen;
class RawDataScreen;
class RawCommandScreen;
class BrowseScreen;
class AddSectionScreen;
class NewRaceScreen;
class RaceStatusScreen;
class GlobalOptionsScreen;
class SkipListScreen;
class ChangeKeyScreen;
class EventLogScreen;
class ProxyOptionsScreen;
class EditProxyScreen;
class ViewFileScreen;
class NukeScreen;
class FileViewerSettingsScreen;
class ScoreBoardScreen;
class SelectSitesScreen;
class TransfersScreen;
class TransferJobStatusScreen;
class AllRacesScreen;
class AllTransferJobsScreen;

class Ui : private EventReceiver, public UIBase {
  private:
    BlockingQueue<UICommand> uiqueue;
    WINDOW * main;
    WINDOW * info;
    WINDOW * legend;
    std::vector<UIWindow *> mainwindows;
    UIWindow * topwindow;
    InfoWindow * infowindow;
    LegendWindow * legendwindow;
    LoginScreen * loginscreen;
    NewKeyScreen * newkeyscreen;
    MainScreen * mainscreen;
    ConfirmationScreen * confirmationscreen;
    EditSiteScreen * editsitescreen;
    SiteStatusScreen * sitestatusscreen;
    RawDataScreen * rawdatascreen;
    RawCommandScreen * rawcommandscreen;
    BrowseScreen * browsescreen;
    AddSectionScreen * addsectionscreen;
    NewRaceScreen * newracescreen;
    RaceStatusScreen * racestatusscreen;
    GlobalOptionsScreen * globaloptionsscreen;
    SkipListScreen * skiplistscreen;
    ChangeKeyScreen * changekeyscreen;
    EventLogScreen * eventlogscreen;
    ProxyOptionsScreen * proxyoptionsscreen;
    EditProxyScreen * editproxyscreen;
    ViewFileScreen * viewfilescreen;
    NukeScreen * nukescreen;
    FileViewerSettingsScreen * fileviewersettingsscreen;
    ScoreBoardScreen * scoreboardscreen;
    SelectSitesScreen * selectsitesscreen;
    TransfersScreen * transfersscreen;
    TransferJobStatusScreen * transferjobstatusscreen;
    AllRacesScreen * allracesscreen;
    AllTransferJobsScreen * alltransferjobsscreen;
    int mainrow;
    int maincol;
    int col;
    int row;
    bool initret;
    bool legendenabled;
    bool infoenabled;
    bool dead;
    bool showlegend;
    bool split;
    std::string eventtext;
    pthread_t uithread;
    Semaphore eventcomplete;
    std::list<UIWindow *> history;
    void FDData();
    void refreshAll();
    void initIntern();
    void enableInfo();
    void disableInfo();
    void enableLegend();
    void disableLegend();
    void redrawAll();
    void switchToWindow(UIWindow *);
    static void * run(void *);
    void tick(int);
    void globalKeyBinds(int);
    void switchToLast();
  public:
    Ui();
    void runInstance();
    bool init();
    void backendPush();
    void terminalSizeChanged();
    void kill();
    void resizeTerm();
    void readConfiguration();
    void writeState();
    bool legendEnabled() const;
    void showLegend(bool);
    void returnToLast();
    void update();
    void setLegend();
    void setInfo();
    void setSplit(bool);
    void redraw();
    void erase();
    void erase(WINDOW *);
    void showCursor();
    void hideCursor();
    void moveCursor(unsigned int, unsigned int);
    void highlight(bool);
    void printStr(unsigned int, unsigned int, std::string);
    void printStr(WINDOW *, unsigned int, unsigned int, std::string);
    void printStr(unsigned int, unsigned int, std::string, bool);
    void printStr(unsigned int, unsigned int, std::string, unsigned int);
    void printStr(unsigned int, unsigned int, std::string, unsigned int, bool);
    void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int, bool);
    void printStr(unsigned int, unsigned int, std::string, unsigned int, bool, bool);
    void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int, bool, bool);
    void printChar(unsigned int, unsigned int, unsigned int);
    void printChar(WINDOW *, unsigned int, unsigned int, unsigned int);
    void printChar(unsigned int, unsigned int, unsigned int, bool);
    void printChar(WINDOW *, unsigned int, unsigned int, unsigned int, bool);
    void goRawCommand(std::string);
    void goRawCommand(std::string, std::string);
    void goConfirmation(std::string);
    void goNuke(std::string, std::string, FileList *);
    void goViewFile(std::string, std::string, FileList *);
    void goAddSection(std::string, std::string);
    void goNewRace(std::string, std::string, std::string);
    void goSelectSites(std::string, std::list<Site *>, std::list<Site *>);
    void goSkiplist();
    void goChangeKey();
    void goProxy();
    void goFileViewerSettings();
    void goSiteStatus(std::string);
    void goRaceStatus(std::string);
    void goTransferJobStatus(std::string);
    void goGlobalOptions();
    void goEventLog();
    void goScoreBoard();
    void goTransfers();
    void goEditSite(std::string);
    void goAddSite();
    void goBrowse(std::string);
    void goAddProxy();
    void goEditProxy(std::string);
    void goRawData(std::string);
    void goRawDataJump(std::string, int);
    void goAllRaces();
    void goAllTransferJobs();
    void returnSelectSites(std::string);
    void key(std::string);
    void newKey(std::string);
    void confirmYes();
    void confirmNo();
    void returnNuke(int);
    void returnRaceStatus(std::string);
};

