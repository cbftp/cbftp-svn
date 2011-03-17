#pragma once
#include <string>
#include "filelist.h"

class SiteRace {
  private:
    std::string section;
    std::string release;
    std::string path;
    FileList * filelist;
  public:
    std::string getSection();
    std::string getRelease();
    std::string getPath();
    SiteRace(std::string, std::string, std::string);
    FileList * getFileList();
};
