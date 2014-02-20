#pragma once

#include <string>
#include <map>
#include <list>

class Race;
class FileList;

class SiteRace {
  private:
    Race * race;
    std::string section;
    std::string release;
    std::string path;
    std::string group;
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
    std::string getGroup();
    std::string getPath();
    std::string getRelevantSubPath();
    SiteRace(Race *, std::string, std::string, std::string);
    ~SiteRace();
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
    void abort();
    void subPathComplete(FileList *);
    bool isSubPathComplete(std::string);
    bool isSubPathComplete(FileList *);
    void reportSize(FileList *, std::list<std::string> *, bool);
    int getObservedTime(FileList *);
    int getSFVObservedTime(FileList *);
    bool hasBeenUpdatedSinceLastCheck();
};
