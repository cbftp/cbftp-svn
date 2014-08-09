#pragma once

#include <string>
#include <list>

#include "eventreceiver.h"

#define SPREAD 0
#define POKEINTERVAL 1000
#define MAXCHECKSTIMEOUT 120

class GlobalContext;
class Race;
class SiteRace;
class FileList;
class File;
class ScoreBoard;

extern GlobalContext * global;

class Engine : public EventReceiver {
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
    int calculateScore(File *, Race *, FileList *, SiteRace *, FileList *, SiteRace *, int, bool *, bool) const;
    bool pokeregistered;
    unsigned int dropped;
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    void removeSiteFromRace(std::string, std::string);
    void abortRace(std::string);
    void someRaceFileListRefreshed();
    int currentRaces() const;
    int allRaces() const;
    Race * getRace(std::string) const;
    std::list<Race *>::iterator getRacesIteratorBegin();
    std::list<Race *>::iterator getRacesIteratorEnd();
    std::list<Race *>::const_iterator getRacesIteratorBegin() const;
    std::list<Race *>::const_iterator getRacesIteratorEnd() const;
    void tick(int);
    void issueGlobalComplete(Race *);
    ScoreBoard * getScoreBoard() const;
};
