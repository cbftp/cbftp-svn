#pragma once

#include <string>
#include <map>

#include "filelist.h"
#include "file.h"
#include "race.h"

class SiteRace {
  private:
    Race * race;
    std::string section;
    std::string release;
    std::string path;
    std::string username;
    std::list<std::string> recentlyvisited;
    std::map<std::string, FileList *> filelists;
    bool done;
    bool sizeestimated;
  public:
    std::string getSection();
    std::string getRelease();
    std::string getPath();
    std::string getLeastRecentlyVisitedSubPath();
    SiteRace(Race *, std::string, std::string, std::string);
    FileList * getFileListForPath(std::string);
    FileList * getFileListForFullPath(std::string);
    std::map<std::string, FileList *>::iterator fileListsBegin();
    std::map<std::string, FileList *>::iterator fileListsEnd();
    Race * getRace();
    void addSubDirectory(std::string);
    std::string getSubPath(FileList *);
    void updateNumFilesUploaded();
    void addNewDirectories();
    bool sizeEstimated();
    int getNumUploadedFiles();
    unsigned long long int getMaxFileSize();
    bool isDone();
    void complete();
    void reportSize(unsigned int);
};
