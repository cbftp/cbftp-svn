#include "potentiallistelement.h"

#include "potentialelement.h"

PotentialListElement::PotentialListElement(int numslots) {
  for (int i = 0; i < numslots; i++) {
    slots.push_back(new PotentialElement());
  }
}

void PotentialListElement::update(SiteLogic * dst, int threads, int dstdnslots, int potential, std::string filename) {
  bool allthreadsused = allThreadsUsedForSite(dst, threads);
  PotentialElement * lowelem = NULL;
  int lowest;
  bool initialized = false;
  for (unsigned int i = 0; i < slots.size(); i++) {
    if (allthreadsused && slots[i]->getSite() != dst) continue;
    if (!initialized || slots[i]->getPotential() < lowest) {
      lowest = slots[i]->getPotential();
      lowelem = slots[i];
      if (!initialized) initialized = true;
    }
  }
  if (lowelem) {
    lowelem->update(dst, dstdnslots, potential, filename);
  }
}

std::vector<PotentialElement *> & PotentialListElement::getSlotsVector() {
  return slots;
}

void PotentialListElement::reset() {
  for (unsigned int i = 0; i < slots.size(); i++) {
    slots[i]->update(NULL, 0, 0, "");
  }
}

bool PotentialListElement::allThreadsUsedForSite(SiteLogic * dst, int threads) const {
  int sitematch = 0;
  for (unsigned int i = 0; i < slots.size(); i++) {
    if(slots[i]->getSite() == dst) {
      sitematch++;
    }
  }
  if (sitematch == threads) return true;
  return false;
}
