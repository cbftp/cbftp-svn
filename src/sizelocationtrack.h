#pragma once

#include <map>

class SiteRace;

class SizeLocationTrack {
private:
  std::map<SiteRace *, unsigned long long int> sitesizes;
  unsigned long long int estimatedsize;
  void recalculate();
public:
  SizeLocationTrack();
  unsigned long long int getEstimatedSize() const;
  int numSites() const;
  bool add(SiteRace *, unsigned long long int);
  void remove(SiteRace *);
};
