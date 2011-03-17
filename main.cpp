#include "main.h"

int main(int argc, char * argv[]) {
  forever = true;
  if (argc < 4) {
    std::cout << "Usage:\n./clusterbomb <release> <section> <site1> <site2> [site3 [site4 [ ... ] ] ]" << std::endl;
    exit(0);
  }
  global = new GlobalContext();
  global->linkManagers(new SiteManager(), new SiteThreadManager(), new TransferManager());
  Engine * e = new Engine();
  std::list<std::string> sites;
  for (int i = 3; i < argc; i++) {
    sites.push_back(argv[i]);
  }
  e->newRace(argv[1], argv[2], sites);
  
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  while(forever) {
    sleep(1);
  }
}

void sighandler(int sig) {
  std::cout << "Saving data to file..." << std::endl;
  global->getSiteManager()->writeDataFile();
  std::cout << "Done, exiting..." << std::endl << std::flush;
  forever = false;
}
