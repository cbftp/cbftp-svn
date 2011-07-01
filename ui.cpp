#include "ui.h"

UserInterface::UserInterface() {
  sem_init(&action, 0, 0);
  sem_init(&initdone, 0, 0);
  loginscreen = NULL;
  mainscreen = NULL;
  confirmationscreen = NULL;
  editsitescreen = NULL;
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
}

bool UserInterface::init() {
  initret = true;
  sem_post(&action);
  sem_wait(&initdone);
  return initret;
}

void UserInterface::initIntern() {
  initscr();
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    kill();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    initret = false;
  }
  sem_post(&initdone);
  loginScreen();
}

void UserInterface::kill() {
  endwin();
}

void UserInterface::putTopRefresh(WINDOW * window) {
  touchwin(window);
  wnoutrefresh(window);
  doupdate();
}

void UserInterface::loginScreen() {
  loginscreen = newwin(row, col, 0, 0);
  std::string svnstring = " This is Project Clusterbomb SVN r" + std::string(SVNREV) + " ";
  std::string compilestring = " Compiled: " + std::string(BUILDTIME) + " ";
  mvwhline(loginscreen, 0, 0, 0, col);
  mvwhline(loginscreen, row-1, 0, 0, col-29);
  mvwhline(loginscreen, row-4, col-29, 0, 29);
  mvwvline(loginscreen, row-4, col-29, 0, 4);
  mvwaddch(loginscreen, row-4, col-29, 4194412);
  mvwaddch(loginscreen, row-1, col-29, 4194410);
  mvwprintw(loginscreen, row-3, col-27, "AES passphrase required:");
  mvwprintw(loginscreen, 0, 3, svnstring.c_str());
  mvwprintw(loginscreen, 0, col - compilestring.length() - 3, compilestring.c_str());
  mvwprintw(loginscreen, row-9, 1, "                              \\         .  ./");
  mvwprintw(loginscreen, row-8, 1, "                           \\      .:\";'.:..\"   /");
  mvwprintw(loginscreen, row-7, 1, "                               (M^^.^~~:.'\").");
  mvwprintw(loginscreen, row-6, 1, "                         -   (/  .    . . \\ \\)  -");
  mvwprintw(loginscreen, row-5, 1, "  O                         ((| :. ~ ^  :. .|))");
  mvwprintw(loginscreen, row-4, 1, " |\\\\                     -   (\\- |  \\ /  |  /)  -");
  mvwprintw(loginscreen, row-3, 1, " |  T                         -\\  \\     /  /-");
  mvwprintw(loginscreen, row-2, 1, "/ \\[_]..........................\\  \\   /  /");
  putTopRefresh(loginscreen);
  std::string key = getStringField(loginscreen, row-2, col-27, "", 25, 32, true);
  // insert decryption stuff here
  mainScreen();
}

void UserInterface::mainScreen() {
  if (mainscreen == NULL) {
    mainscreen = newwin(row, col, 0, 0);
  }
  MenuSelectSite mss(mainscreen);
  bool redraw = false;
  while (true) {
    werase(mainscreen);
    mvwprintw(mainscreen, 1, 1, "-=== MAIN SCREEN ===-");
    mvwprintw(mainscreen, 3, 1, std::string("Sites added: " + global->int2Str(global->getSiteManager()->getNumSites())).c_str());
    mvwprintw(mainscreen, 5, 1, "Site    Logins  Uploads  Downloads");
    int x = 1;
    int y = 7;
    mss.prepareRefill();
    for (std::map<std::string, Site *>::iterator it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
      mss.add(it->second, y++, x);
    }
    mss.print();
    putTopRefresh(mainscreen);
    keypad(mainscreen, TRUE);
    redraw = false;
    Site * site;
    while(!redraw) {
      switch(wgetch(mainscreen)) {
        case KEY_UP:
          if (mss.getSite() == NULL) break;
          mss.goPrev();
          break;
        case KEY_DOWN:
          if (mss.getSite() == NULL) break;
          mss.goNext();
          break;
        case 10:
          if (mss.getSite() == NULL) break;
          siteStatusScreen(mss.getSite());
          putTopRefresh(mainscreen);
          break;
        case 'E':
          if (mss.getSite() == NULL) break;
          if (editSiteScreen(mss.getSite())) redraw = true;
          else putTopRefresh(mainscreen);
          break;
        case 'A':
          site = new Site("SUNET");
          if (editSiteScreen(site)) {
            redraw = true;
            global->getSiteManager()->addSite(site);
          }
          else {
            delete site;
            putTopRefresh(mainscreen);
          }
          break;
        case 'C':
          if (mss.getSite() == NULL) break;
          site = new Site(*mss.getSite());
          int i;
          for (i = 0; global->getSiteManager()->getSite(site->getName() + "-" + global->int2Str(i)) != NULL; i++);
          site->setName(site->getName() + "-" + global->int2Str(i));
          global->getSiteManager()->addSite(site);
          redraw = true;
          break;
        case 'D':
          if (mss.getSite() == NULL) break;
          if (confirmationScreen()) {
            redraw = true;
            global->getSiteThreadManager()->deleteSiteThread(mss.getSite()->getName());
            global->getSiteManager()->deleteSite(mss.getSite()->getName());
          }
          else putTopRefresh(mainscreen);
          break;
      }
    }
  }
}

int UserInterface::editSiteScreen(Site * site) {
  if (editsitescreen == NULL) {
    editsitescreen = newwin(row, col, 0, 0);
  }
  else wclear(editsitescreen);
  int y = 1;
  int x = 1;
  mvwprintw(editsitescreen, y++, x, std::string("Name: " + site->getName()).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Address: " + site->getAddress()).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Port: " + site->getPort()).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Username: " + site->getUser()).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Password: " + site->getPass()).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Login slots: " + global->int2Str(site->getMaxLogins())).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Upload slots: " + global->int2Str(site->getMaxUp())).c_str());
  mvwprintw(editsitescreen, y++, x, std::string("Download slots: " + global->int2Str(site->getMaxDown())).c_str());
  putTopRefresh(editsitescreen);
  while(true) {
    switch(wgetch(editsitescreen)) {
      case 10:
        return 1;
    }
  }
}

void UserInterface::siteStatusScreen(Site * site) {
  WINDOW * window;
  std::map<std::string, WINDOW *>::iterator it = sitestatusscreen.find(site->getName());
  if (it != sitestatusscreen.end()) {
    window = it->second;
  }
  else {
    window = sitestatusscreen[site->getName()] = newwin(row, col, 0, 0);
    mvwprintw(window, 1, 1, std::string("Detailed status for " + site->getName()).c_str());
  }
  putTopRefresh(window);
}

int UserInterface::confirmationScreen() {
  if (confirmationscreen == NULL) {
    confirmationscreen = newwin(row, col, 0, 0);
  }
  mvwprintw(confirmationscreen, 1, 1, "CONFIRM YOUR CHOICE");
  mvwprintw(confirmationscreen, 3, 1, "Are you sure (y/N)? ");
  putTopRefresh(confirmationscreen);
  while(1) {
    switch(wgetch(confirmationscreen)) {
      case 'y':
        return 1;
      case 'n':
      case 10:
        return 0;
    }
  }
}

std::string UserInterface::getStringField(WINDOW * window, int row, int col, std::string startstr, int fieldlen, int maxlen, bool secret) {
  global->signal_ignore();
  std::string str = startstr;
  int ch;
  int ccol = col + startstr.length();
  keypad(window, TRUE);
  noecho();
  while(ch = mvwgetch(window, row, ccol)) {
    if (ch >= 32 && ch <= 126) {
      if (str.length() == maxlen) continue;
      str += (char)ch;
      if (str.length() <= fieldlen) {
        mvwaddch(window, row, ccol++, secret ? '*' : ch);
      }
      else {
        int start = str.length() - fieldlen;
        for (int i = 0; i < fieldlen; i++) {
          mvwaddch(window, row, col+i, secret ? '*' : str[start+i]);
        }
      }
      wrefresh(window);
    }
    else {
      switch(ch) {
        case KEY_BACKSPACE:
          if (str.length() == 0) continue;
          str = str.substr(0, str.length()-1);
          if (str.length() < fieldlen) {
            mvwaddch(window, row, --ccol, ' ');
          }
          else {
            int start = str.length() - fieldlen;
            for (int i = 0; i < fieldlen; i++) {
              mvwaddch(window, row, col+i, secret ? '*' : str[start+i]);
            }
          }
          wrefresh(window);
          break;
        case KEY_ENTER:
        case 10:
        case 13:
          global->signal_catch();
          return str;
          break;
      }
    }
  }
}

void UserInterface::runInstance() {
  while(1) {
    sem_wait(&action);
    initIntern();
  }
}

void * UserInterface::run(void * arg) {
  ((UserInterface *) arg)->runInstance();
}
