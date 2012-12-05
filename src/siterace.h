#pragma once

#include <string>

#include "filelist.h"
#include "race.h"

class SiteRace {
  private:
    Race * race;
    std::string section;
    std::string release;
    std::string path;
    FileList * filelist;
    bool done;
    bool sizeestimated;
  public:
    std::string getSection();
    std::string getRelease();
    std::string getPath();
    SiteRace(Race *, std::string, std::string, std::string);
    FileList * getFileList();
    Race * getRace();
    void updateNumFilesUploaded();
    bool sizeEstimated();
    bool isDone();
    void complete();
    void reportSize(unsigned int);
};
