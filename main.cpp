#include "main.h"

Main::Main() {
  std::string datadirpath = std::string(getenv("HOME")) + "/" + DATAPATH;
  std::string datapath = datadirpath + "/" + DATAFILE;
  char * specialdatapath = getenv("CLUSTERBOMB_DATA_PATH");
  if (specialdatapath != NULL) {
    datapath = std::string(specialdatapath);
  }
  else if (access(datadirpath.c_str(), F_OK) == 0) {
    if (access(datadirpath.c_str(), R_OK | W_OK) < 0) {
      perror(std::string("Error: could not access " + datadirpath).c_str());
      exit(1);
    }
    struct stat status;
    stat(datadirpath.c_str(), &status);
    if (!(status.st_mode & S_IFDIR)) {
      std::cout << "Error: could not create " << datadirpath << ": File exists" << std::endl;
      exit(1);
    }
  }
  else {
    if (mkdir(datadirpath.c_str(), 0700) < 0) {
      perror(std::string("Error: could not create " + datadirpath).c_str());
      exit(1);
    }
  }
  forever = true;
  global = new GlobalContext();
  DataFileHandler * dfh = new DataFileHandler(datapath);
  Engine * e = new Engine();
  UserInterface * ui = new UserInterface();
  SiteManager * sm = new SiteManager();
  SiteThreadManager * stm = new SiteThreadManager();
  TransferManager * tm = new TransferManager();
  global->linkComponents(dfh, e, ui->getCommunicator(), sm, stm, tm);
  if (!ui->init()) exit(1);
  while(forever) {
    sleep(1);
  }
}

int main(int argc, char * argv[]) {
  /*if (argc < 4) {
    std::cout << "Usage:\n./clusterbomb <release> <section> <site1> <site2> [site3 [site4 [ ... ] ] ]" << std::endl;
    exit(0);
  }
  std::list<std::string> sites;
  for (int i = 3; i < argc; i++) {
    sites.push_back(argv[i]);
  }
  e->newRace(argv[1], argv[2], sites);*/

  global->signal_catch();
  new Main();
}

void sighandler(int sig) {
  global->signal_ignore();
  global->getUICommunicator()->kill();
  if (global->getDataFileHandler()->isInitialized()) {
    std::cout << "Saving data to file..." << std::endl;
    global->getSiteManager()->writeState();
    global->getDataFileHandler()->writeFile();
    std::cout << "Done, exiting..." << std::endl << std::flush;
  }
  forever = false;
}

void sighandler_winch(int sig) {
  signal(SIGWINCH, &sighandler_ignore);
	global->getUICommunicator()->terminalSizeChanged();
	signal(SIGWINCH, &sighandler_winch);
}

void sighandler_ignore(int sig) {
}

