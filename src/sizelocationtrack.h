#pragma once

#include <map>
#include <string>

class SiteRace;

class SizeLocationTrack {
private:
  std::map<SiteRace *, unsigned long long int> sitesizes;
  unsigned long long int estimatedsize;
  void recalculate();
public:
  SizeLocationTrack();
  unsigned long long int getEstimatedSize();
  int numSites();
  void add(SiteRace *, unsigned long long int);
  void remove(SiteRace *);
};
