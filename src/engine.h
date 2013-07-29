#pragma once

#include <string>
#include <list>
#include <stdlib.h>

#include "scoreboard.h"
#include "scoreboardelement.h"
#include "globalcontext.h"
#include "sitelogic.h"
#include "filelist.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "race.h"
#include "enginebase.h"

#define SPREAD 1

extern GlobalContext * global;

class Engine : public EngineBase {
  private:
    std::list<Race *> allraces;
    std::list<Race *> currentraces;
    ScoreBoard * scoreboard;
    int maxavgspeed;
    void estimateRaceSizes();
    void reportCurrentSizeAsFinal(SiteRace *, FileList *);
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    int calculateScore(File *, Race *, FileList *, SiteRace *, FileList *, SiteRace *, int, bool);
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    void someRaceFileListRefreshed();
    int currentRaces();
    int allRaces();
    Race * getRace(std::string);
    std::list<Race *>::iterator getRacesIteratorBegin();
    std::list<Race *>::iterator getRacesIteratorEnd();
};
