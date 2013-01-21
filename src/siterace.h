#pragma once

#include <string>
#include <map>

#include "filelist.h"
#include "file.h"
#include "race.h"
#include "globalcontext.h"

extern GlobalContext * global;

class SiteRace {
  private:
    Race * race;
    std::string section;
    std::string release;
    std::string path;
    std::string username;
    std::list<std::string> recentlyvisited;
    std::list<std::string> completesubdirs;
    std::map<std::string, FileList *> filelists;
    bool done;
    std::list<FileList *> sizeestimated;
    std::map<FileList *, int> observestarts;
    std::map<FileList *, int> sfvobservestarts;
  public:
    std::string getSection();
    std::string getRelease();
    std::string getPath();
    std::string getRelevantSubPath();
    SiteRace(Race *, std::string, std::string, std::string);
    FileList * getFileListForPath(std::string);
    FileList * getFileListForFullPath(std::string);
    std::string getSubPathForFileList(FileList *);
    std::map<std::string, FileList *>::iterator fileListsBegin();
    std::map<std::string, FileList *>::iterator fileListsEnd();
    Race * getRace();
    void addSubDirectory(std::string);
    std::string getSubPath(FileList *);
    void updateNumFilesUploaded();
    void addNewDirectories();
    bool sizeEstimated(FileList *);
    int getNumUploadedFiles();
    unsigned long long int getMaxFileSize();
    bool isDone();
    void complete();
    void subPathComplete(FileList *);
    bool isSubPathComplete(std::string);
    void reportSize(FileList *, unsigned int);
    int getObservedTime(FileList *);
    int getSFVObservedTime(FileList *);
};
