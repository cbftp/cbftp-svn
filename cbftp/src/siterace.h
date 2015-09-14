#pragma once

#include <string>
#include <map>
#include <list>

#include "commandowner.h"
#include "pointer.h"

class Race;
class FileList;

class SiteRace : public CommandOwner {
  private:
    Pointer<Race> race;
    std::string section;
    std::string release;
    std::string path;
    std::string group;
    std::string username;
    std::string sitename;
    std::list<std::string> recentlyvisited;
    std::list<std::string> completesubdirs;
    std::map<std::string, FileList *> filelists;
    bool done;
    std::list<FileList *> sizeestimated;
    std::map<FileList *, unsigned long long int> observestarts;
    std::map<FileList *, unsigned long long int> sfvobservestarts;
    std::map<std::string, bool> visitedpaths;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int numuploadedfiles;
    void updateNumFilesUploaded();
    void addNewDirectories();
  public:
    int classType() const;
    std::string getSiteName() const;
    std::string getSection() const;
    std::string getRelease() const;
    std::string getGroup() const;
    std::string getPath() const;
    std::string getRelevantSubPath();
    SiteRace(Pointer<Race>, std::string, std::string, std::string, std::string);
    ~SiteRace();
    FileList * getFileListForPath(std::string) const;
    FileList * getFileListForFullPath(std::string) const;
    std::string getSubPathForFileList(FileList *) const;
    std::map<std::string, FileList *>::const_iterator fileListsBegin() const;
    std::map<std::string, FileList *>::const_iterator fileListsEnd() const;
    Pointer<Race> getRace() const;
    void addSubDirectory(std::string);
    std::string getSubPath(FileList *) const;
    void fileListUpdated(FileList *);
    bool sizeEstimated(FileList *) const;
    unsigned int getNumUploadedFiles() const;
    unsigned long long int getMaxFileSize() const;
    unsigned long long int getTotalFileSize() const;
    bool isDone() const;
    bool isGlobalDone() const;
    void complete(bool);
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
