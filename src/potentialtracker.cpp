#include "potentialtracker.h"

PotentialTracker::PotentialTracker(int slots) {
  for (int i = 0; i < POTENTIALITY_SLICES; i++) {
    potentiallist.push_back(new PotentialListElement(slots));
  }
  for (int i = 0; i < slots; i++) top.push_back(new PotentialElement());
  pthread_mutex_init(&listmutex, NULL);
  sem_init(&tick, 0, 0);
  global->getTickPoke()->startPoke(&tick, POTENTIALITY_LIFESPAN * 1000 / POTENTIALITY_SLICES, 0);
  pthread_create(&thread, global->getPthreadAttr(), runPotentialTracker, (void *) this);
  pthread_setname_np(thread, "PotentialTrackerThread");
}

int PotentialTracker::getMaxAvailablePotential() {
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    (*ittop)->update(NULL, 0, 0, "");
  }
  pthread_mutex_lock(&listmutex);
  for (itple = potentiallist.begin(); itple != potentiallist.end(); itple++) {
    std::vector<PotentialElement *> & pelist = (*itple)->getSlotsVector();
    for (itpe = pelist.begin(); itpe != pelist.end(); itpe++) {
      PotentialElement * mpe;
      std::list<PotentialElement *>::iterator itrpe = top.begin();
      if ((*itpe)->getSite() != NULL && allTopSlotsUsedForSite(*itpe)) {
        itrpe = findFirstOfSite((*itpe)->getSite());
      }
      mpe = *itrpe;
      bool inserted = false;
      bool duplicate = false;
      bool lowscore = false;
      for (ittop = top.begin(); ittop != top.end(); ittop++) {
        if ((*itpe)->getPotential() < (*ittop)->getPotential()) {
          if (ittop == top.begin()) {
            lowscore = true;
            break;
          }
          for (ittop2 = top.begin(); ittop2 != top.end(); ittop2++) {
            if ((*itpe)->getSite() == (*ittop2)->getSite() && (*itpe)->getFileName().compare((*ittop2)->getFileName()) == 0) {
              duplicate = true;
              break;
            }
          }
          if (duplicate) break;
          top.insert(ittop, mpe);
          top.erase(itrpe);
          mpe->update((*itpe)->getSite(), (*itpe)->getSiteDownloadSlots(), (*itpe)->getPotential(), (*itpe)->getFileName());
          inserted = true;
          break;
        }
      }
      if (!lowscore && !duplicate && !inserted && (*itpe)->getPotential() >= top.back()->getPotential()) {
        top.push_back(mpe);
        top.erase(itrpe);
        mpe->update((*itpe)->getSite(), (*itpe)->getSiteDownloadSlots(), (*itpe)->getPotential(), (*itpe)->getFileName());
      }
    }
  }
  int ret = top.front()->getPotential();
  pthread_mutex_unlock(&listmutex);
  return ret;
}

PotentialListElement * PotentialTracker::getFront() {
  pthread_mutex_lock(&listmutex);
  PotentialListElement * ple = potentiallist.front();
  pthread_mutex_unlock(&listmutex);
  return ple;
}

void * runPotentialTracker(void * arg) {
  ((PotentialTracker *) arg)->runInstance();
  return NULL;
}

void PotentialTracker::runInstance() {
  while(1) {
    sem_wait(&tick);
    pthread_mutex_lock(&listmutex);
    potentiallist.back()->reset();
    potentiallist.push_front(potentiallist.back());
    potentiallist.pop_back();
    pthread_mutex_unlock(&listmutex);
  }
}

std::list<PotentialElement *>::iterator PotentialTracker::findFirstOfSite(SiteThread * st) {
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if ((*ittop)->getSite() == st) return ittop;
  }
  return top.end();
}

bool PotentialTracker::allTopSlotsUsedForSite(PotentialElement * pe) {
  int threads = pe->getSiteDownloadSlots();
  int sitematch = 0;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if((*ittop)->getSite() == pe->getSite()) {
      sitematch++;
    }
  }
  if (sitematch == threads) return true;
  return false;
}
