#include "potentialtracker.h"

#include "globalcontext.h"
#include "potentialelement.h"
#include "potentiallistelement.h"
#include "tickpoke.h"

PotentialTracker::PotentialTracker(int slots) {
  for (int i = 0; i < POTENTIALITY_SLICES; i++) {
    potentiallist.push_back(new PotentialListElement(slots));
  }
  for (int i = 0; i < slots; i++) top.push_back(new PotentialElement());
  global->getTickPoke()->startPoke(this, "PotentialTracker", POTENTIALITY_LIFESPAN / POTENTIALITY_SLICES, 0);
}

PotentialTracker::~PotentialTracker() {
  global->getTickPoke()->stopPoke(this, 0);
}

int PotentialTracker::getMaxAvailablePotential() {
  std::list<PotentialElement *>::iterator ittop;
  std::list<PotentialElement *>::iterator ittop2;
  std::list<PotentialListElement *>::iterator itple;
  std::vector<PotentialElement *>::iterator itpe;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    (*ittop)->update(NULL, 0, 0, "");
  }
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
  return ret;
}

PotentialListElement * PotentialTracker::getFront() const {
  return potentiallist.front();
}

void PotentialTracker::tick(int message) {
  potentiallist.back()->reset();
  potentiallist.push_front(potentiallist.back());
  potentiallist.pop_back();
}

std::list<PotentialElement *>::iterator PotentialTracker::findFirstOfSite(SiteLogic * st) {
  std::list<PotentialElement *>::iterator ittop;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if ((*ittop)->getSite() == st) return ittop;
  }
  return top.end();
}

bool PotentialTracker::allTopSlotsUsedForSite(PotentialElement * pe) const {
  int threads = pe->getSiteDownloadSlots();
  int sitematch = 0;
  std::list<PotentialElement *>::const_iterator ittop;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if((*ittop)->getSite() == pe->getSite()) {
      sitematch++;
    }
  }
  if (sitematch == threads) return true;
  return false;
}
