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
    std::map<std::string, bool> visitedpaths;
    unsigned long long int maxfilesize;
  public:
    std::string getSection() const;
    std::string getRelease() const;
    std::string getGroup() const;
    std::string getPath() const;
    std::string getRelevantSubPath();
    SiteRace(Race *, std::string, std::string, std::string);
    ~SiteRace();
    FileList * getFileListForPath(std::string) const;
    FileList * getFileListForFullPath(std::string) const;
    std::string getSubPathForFileList(FileList *) const;
    std::map<std::string, FileList *>::const_iterator fileListsBegin() const;
    std::map<std::string, FileList *>::const_iterator fileListsEnd() const;
    Race * getRace() const;
    void addSubDirectory(std::string);
    std::string getSubPath(FileList *) const;
    void updateNumFilesUploaded();
    void addNewDirectories();
    bool sizeEstimated(FileList *) const;
    int getNumUploadedFiles() const;
    unsigned long long int getMaxFileSize() const;
    bool isDone() const;
    void complete();
    void abort();
    void subPathComplete(FileList *);
    bool isSubPathComplete(std::string) const;
    bool isSubPathComplete(FileList *) const;
    void reportSize(FileList *, std::list<std::string> *, bool);
    int getObservedTime(FileList *);
    int getSFVObservedTime(FileList *);
    bool hasBeenUpdatedSinceLastCheck();
    void addVisitedPath(std::string);
    bool pathVisited(std::string) const;
};
