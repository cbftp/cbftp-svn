#pragma once
#include <string>

class SiteThread;
class SiteRace;

class ScoreBoardElement {
  private:
    std::string filename;
    SiteThread * src;
    SiteThread * dst;
    SiteRace * srs;
    SiteRace * srd;
    int score;
  public:
    ScoreBoardElement(std::string, int, SiteThread *, SiteRace *, SiteThread *, SiteRace *);
    std::string fileName();
    SiteThread * getSource();
    SiteThread * getDestination();
    SiteRace * getSourceSiteRace();
    SiteRace * getDestinationSiteRace();
    int getScore();
};
