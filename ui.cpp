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
  curs_set(0);
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
  curs_set(1);
  std::string svnstring = " This is Project Clusterbomb SVN r" + std::string(SVNREV) + " ";
  std::string compilestring = " Compiled: " + std::string(BUILDTIME) + " ";
  int boxchar = 0;
  for(int i = 1; i < row; i++) {
    for(int j = 0; j < col; j++) {
      if(i == 1) boxchar = (i+j)%2==0 ? 4194423 : 4194417;
      else if (i == row-1) {
        if (j < col-29) boxchar = (i+j)%2==0 ? 4194417 : 4194422;
        else if (j == col-29) boxchar = 4194410;
        else continue;
      }
      else if ((i == row-2 || i == row-3) && j >= col-29) {
        if (j == col-29) boxchar = (i+j)%2==0 ? 4194424 : 4194421;
        else continue;
      }
      else if (i == row-4 && j >= col-29) {
        if (j == col-29) boxchar = (i+j)%2==0 ? 4194412 : 4194414;
        else boxchar = (i+j)%2==0 ? 4194417 : 4194422;
      }
      else boxchar = (i+j)%2==0 ? 4194412 : 4194410;
      if (boxchar) mvwaddch(loginscreen, i, j, boxchar);
    }
  }
  mvwprintw(loginscreen, row-3, col-27, "AES passphrase required:");
  mvwprintw(loginscreen, 0, 3, svnstring.c_str());
  mvwprintw(loginscreen, 0, col - compilestring.length() - 3, compilestring.c_str());
  putTopRefresh(loginscreen);
  std::string key = getStringField(loginscreen, row-2, col-27, "", 25, 32, true);
  // insert decryption stuff here
  curs_set(0);
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
    if (global->getSiteManager()->getNumSites()) mvwprintw(mainscreen, 5, 1, "Site    Logins  Uploads  Downloads");
    else mvwprintw(mainscreen, 5, 1, "Press 'A' to add a site");
    int x = 1;
    int y = 7;
    mss.prepareRefill();
    for (std::vector<Site *>::iterator it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
      mss.add(*it, y++, x);
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
            global->getSiteManager()->addSite(site);
            redraw = true;
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
  Site modsite = Site(*site);
  MenuSelectOption mso(editsitescreen);
  bool redraw = false;
  while (true) {
    mso.clear();
    werase(editsitescreen);
    int y = 3;
    int x = 1;
    mvwprintw(editsitescreen, 1, 1, "-== SITE OPTIONS ==-");
    mso.addStringField(y++, x, "name", "Name:", modsite.getName());
    mso.addStringField(y++, x, "addr", "Address:", modsite.getAddress());
    mso.addStringField(y++, x, "port", "Port:", modsite.getPort());
    mso.addStringField(y++, x, "user", "Username:", modsite.getUser());
    mso.addStringField(y++, x, "pass", "Password:", modsite.getPass());
    mso.addIntArrow(y++, x, "logins", "Login slots:", modsite.getMaxLogins());
    mso.addIntArrow(y++, x, "maxup", "Upload slots:", modsite.getMaxUp());
    mso.addIntArrow(y++, x, "maxdn", "Download slots:", modsite.getMaxDown());
    mso.addCheckBox(y++, x, "pret", "Needs PRET:", modsite.needsPRET());
    mso.addCheckBox(y++, x, "brokenpasv", "Broken PASV:", modsite.hasBrokenPASV());
    mso.print();
    keypad(editsitescreen, TRUE);
    putTopRefresh(editsitescreen);
    redraw = false;
    int datacol;
    int datarow;
    std::string id;
    while(!redraw) {
      switch(wgetch(editsitescreen)) {
        case KEY_UP:
          mso.goPrev();
          break;
        case KEY_DOWN:
          mso.goNext();
          break;
        case 32:
        case 10:
          datacol = mso.getSelectionDataCol();
          datarow = mso.getSelectionDataRow();
          id = mso.getSelection().getIdentifier();
          if (id.compare("name") == 0) {
            modsite.setName(getStringField(editsitescreen, datarow, datacol, mso.getSelection().getContent(), 10, 10, false));
          }
          else if (id.compare("addr") == 0) {
            modsite.setAddress(getStringField(editsitescreen, datarow, datacol, mso.getSelection().getContent(), 50, 50, false));
          }
          else if (id.compare("port") == 0) {
            modsite.setPort(getStringField(editsitescreen, datarow, datacol, mso.getSelection().getContent(), 5, 5, false));
          }
          else if (id.compare("user") == 0) {
            modsite.setUser(getStringField(editsitescreen, datarow, datacol, mso.getSelection().getContent(), 32, 32, false));
          }
          else if (id.compare("pass") == 0) {
            modsite.setPass(getStringField(editsitescreen, datarow, datacol, mso.getSelection().getContent(), 32, 32, false));
          }
          else if (id.compare("logins") == 0) {
            modsite.setMaxLogins(getNumArrow(editsitescreen, datarow, datacol, mso.getSelection().getIntContent()));
          }
          else if (id.compare("maxup") == 0) {
            modsite.setMaxUp(getNumArrow(editsitescreen, datarow, datacol, mso.getSelection().getIntContent()));
          }
          else if (id.compare("maxdn") == 0) {
            modsite.setMaxDn(getNumArrow(editsitescreen, datarow, datacol, mso.getSelection().getIntContent()));
          }
          else if (id.compare("pret") == 0) {
            modsite.setPRET(getCheckBoxBool(editsitescreen, datarow, datacol, mso.getSelection().getIntContent()));
          }
          else if (id.compare("brokenpasv") == 0) {
            modsite.setBrokenPASV(getCheckBoxBool(editsitescreen, datarow, datacol, mso.getSelection().getIntContent()));
          }

          redraw = true;
          break;
        case 'd':
          site->copy(&modsite);
          return 1;
        case 'c':
          return 0;
      }
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
  curs_set(1);
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
          curs_set(0);
          return str;
          break;
      }
    }
  }
}

int UserInterface::getNumArrow(WINDOW * window, int row, int col, int startval) {
  int value = startval;
  mvwprintw(window, row, col, "< ");
  bool done = false;
  while(!done) {
    std::string outstr = global->int2Str(value) + " >";
    mvwprintw(window, row, col + 2, "     ");
    mvwprintw(window, row, col + 2, outstr.c_str());
    switch(wgetch(window)) {
      case KEY_DOWN:
      case KEY_LEFT:
        if (value > 0) value--;
        break;
      case KEY_UP:
      case KEY_RIGHT:
        if (value < 99) value++;
        break;
      case 10:
      case 32:
        done = true;
        break;
    }
  }
  return value;
}

bool UserInterface::getCheckBoxBool(WINDOW * window, int row, int col, int startval) {
  if (startval) {
    mvwprintw(window, row, col, "[ ]");
    return false;
  }
  mvwprintw(window, row, col, "[X]");
  return true;
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
