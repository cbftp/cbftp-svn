#include "ui.h"

UserInterface::UserInterface() {
  sem_init(&action, 0, 0);
  sem_init(&initdone, 0, 0);
  loginscreen = NULL;
  mainscreen = NULL;
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

void UserInterface::refreshFront() {
  wrefresh(front);
}

void UserInterface::loginScreen() {
  loginscreen = newwin(row, col, 0, 0);
  mvwhline(loginscreen, row-4, col-29, 0, 29);
  mvwvline(loginscreen, row-4, col-29, 0, 4);
  mvwaddch(loginscreen, row-4, col-29, 4194412);
  mvwprintw(loginscreen, row-3, col-27, "AES passphrase required:");
  mvwprintw(loginscreen, 1, 1, std::string("This is Project Clusterbomb SVN r" + std::string(SVNREV)).c_str());
  mvwprintw(loginscreen, 2, 1, std::string("Compiled: " + std::string(BUILDTIME)).c_str());
  mvwprintw(loginscreen, row-8, 1, "                              \\         .  ./");
  mvwprintw(loginscreen, row-7, 1, "                           \\      .:\";'.:..\"   /");
  mvwprintw(loginscreen, row-6, 1, "                               (M^^.^~~:.'\").");
  mvwprintw(loginscreen, row-5, 1, "                         -   (/  .    . . \\ \\)  -");
  mvwprintw(loginscreen, row-4, 1, "  O                         ((| :. ~ ^  :. .|))");
  mvwprintw(loginscreen, row-3, 1, " |\\\\                     -   (\\- |  \\ /  |  /)  -");
  mvwprintw(loginscreen, row-2, 1, " |  T                         -\\  \\     /  /-");
  mvwprintw(loginscreen, row-1, 1, "/ \\[_]..........................\\  \\   /  /");
  front = loginscreen;
  refreshFront();
  std::string key = getStringField(loginscreen, row-2, col-27, "", 25, 32, true);
  // insert decryption stuff here
  mainScreen();
}

void UserInterface::mainScreen() {
  if (mainscreen == NULL) {
    mainscreen = newwin(row, col, 0, 0);
    mvwprintw(mainscreen, 1, 1, "-=== MAIN SCREEN ===-");
    mvwprintw(mainscreen, 3, 1, std::string("Sites added: " + global->int2Str(global->getSiteManager()->getNumSites())).c_str());
    int x = 1;
    int y = 5;
    for (std::map<std::string, Site *>::iterator it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
      mvwprintw(mainscreen, y, x++, it->first.c_str());
    }
  }
  front = mainscreen;
  refreshFront();
}

void UserInterface::siteStatus(Site * site) {
  WINDOW * window;
  std::map<std::string, WINDOW *>::iterator it = sitestatusscreen.find(site->getName());
  if (it != sitestatusscreen.end()) {
    window = it->second;
  }
  else {
    window = sitestatusscreen[site->getName()] = newwin(row, col, 0, 0);
    mvwprintw(window, row/2-5, 0, std::string("Detailed status for " + site->getName()).c_str());
  }
  front = window;
  refreshFront();
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
