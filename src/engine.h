#pragma once

#include <string>
#include <list>

#define SPREAD 0

class GlobalContext;
class Race;
class SiteRace;
class FileList;
class File;
class ScoreBoard;

extern GlobalContext * global;

class Engine {
  private:
    std::list<Race *> allraces;
    std::list<Race *> currentraces;
    ScoreBoard * scoreboard;
    int maxavgspeed;
    void estimateRaceSizes();
    void reportCurrentSize(SiteRace *, FileList *, bool final);
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    int calculateScore(File *, Race *, FileList *, SiteRace *, FileList *, SiteRace *, int, bool);
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    void removeSiteFromRace(std::string, std::string);
    void abortRace(std::string);
    void someRaceFileListRefreshed();
    int currentRaces();
    int allRaces();
    Race * getRace(std::string);
    std::list<Race *>::iterator getRacesIteratorBegin();
    std::list<Race *>::iterator getRacesIteratorEnd();
};
