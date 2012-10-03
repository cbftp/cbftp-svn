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
  public:
    std::string getSection();
    std::string getRelease();
    std::string getPath();
    SiteRace(Race *, std::string, std::string, std::string);
    FileList * getFileList();
    void updateNumFilesUploaded();
    bool isDone();
};
