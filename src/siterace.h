#pragma once

#include <string>
#include <map>
#include <set>
#include <list>
#include <set>

#include "core/pointer.h"
#include "commandowner.h"
#include "path.h"

class Race;
class FileList;
class SiteLogic;
class SkipList;

class SiteRace : public CommandOwner {
  private:
    Pointer<Race> race;
    Path section;
    std::string release;
    Path path;
    std::string group;
    std::string username;
    std::string sitename;
    std::list<std::string> recentlyvisited;
    std::list<std::string> completesubdirs;
    std::map<std::string, FileList *> filelists;
    bool done;
    bool aborted;
    bool donebeforeabort;
    std::set<FileList *> sizeestimated;
    std::map<FileList *, unsigned long long int> observestarts;
    std::map<FileList *, unsigned long long int> sfvobservestarts;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int numuploadedfiles;
    int profile;
    const SkipList & skiplist;
    std::map<std::string, unsigned long long int> sitessizedown;
    std::map<std::string, unsigned int> sitesfilesdown;
    std::map<std::string, unsigned int> sitesspeeddown;
    std::map<std::string, unsigned long long int> sitessizeup;
    std::map<std::string, unsigned int> sitesfilesup;
    std::map<std::string, unsigned int> sitesspeedup;
    unsigned long long int sizedown;
    unsigned int filesdown;
    unsigned int speeddown;
    unsigned long long int sizeup;
    unsigned int filesup;
    unsigned int speedup;
    void updateNumFilesUploaded();
    void addNewDirectories();
    void markNonExistent(FileList *);
    void reset();
  public:
    std::string getName() const;
    int classType() const;
    std::string getSiteName() const;
    const Path & getSection() const;
    std::string getRelease() const;
    std::string getGroup() const;
    const Path & getPath() const;
    unsigned int getId() const;
    std::string getRelevantSubPath();
    bool anyFileListNotNonExistent() const;
    SiteRace(Pointer<Race>, const std::string &, const Path &, const std::string &, const std::string &, const SkipList &);
    ~SiteRace();
    FileList * getFileListForPath(const std::string &) const;
    FileList * getFileListForFullPath(SiteLogic * co, const Path &) const;
    std::string getSubPathForFileList(FileList *) const;
    std::map<std::string, FileList *>::const_iterator fileListsBegin() const;
    std::map<std::string, FileList *>::const_iterator fileListsEnd() const;
    Pointer<Race> getRace() const;
    bool addSubDirectory(const std::string &);
    bool addSubDirectory(const std::string &, bool);
    std::string getSubPath(FileList *) const;
    void fileListUpdated(SiteLogic *, FileList *);
    bool sizeEstimated(FileList *) const;
    unsigned int getNumUploadedFiles() const;
    unsigned long long int getMaxFileSize() const;
    unsigned long long int getTotalFileSize() const;
    bool isDone() const;
    bool isAborted() const;
    bool doneBeforeAbort() const;
    bool isGlobalDone() const;
    int getProfile() const;
    void complete(bool);
    void abort();
    void softReset();
    void hardReset();
    void subPathComplete(FileList *);
    bool isSubPathComplete(const std::string &) const;
    bool isSubPathComplete(FileList *) const;
    void reportSize(FileList *, const std::set<std::string> &, bool);
    int getObservedTime(FileList *);
    int getSFVObservedTime(FileList *);
    bool hasBeenUpdatedSinceLastCheck();
    void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int, unsigned int);
    unsigned long long int getSizeDown() const;
    unsigned int getFilesDown() const;
    unsigned int getSpeedDown() const;
    unsigned long long int getSizeUp() const;
    unsigned int getFilesUp() const;
    unsigned int getSpeedUp() const;
    std::map<std::string, unsigned long long int>::const_iterator sizeUpBegin() const;
    std::map<std::string, unsigned int>::const_iterator filesUpBegin() const;
    std::map<std::string, unsigned int>::const_iterator speedUpBegin() const;
    std::map<std::string, unsigned long long int>::const_iterator sizeDownBegin() const;
    std::map<std::string, unsigned int>::const_iterator filesDownBegin() const;
    std::map<std::string, unsigned int>::const_iterator speedDownBegin() const;
    std::map<std::string, unsigned long long int>::const_iterator sizeUpEnd() const;
    std::map<std::string, unsigned int>::const_iterator filesUpEnd() const;
    std::map<std::string, unsigned int>::const_iterator speedUpEnd() const;
    std::map<std::string, unsigned long long int>::const_iterator sizeDownEnd() const;
    std::map<std::string, unsigned int>::const_iterator filesDownEnd() const;
    std::map<std::string, unsigned int>::const_iterator speedDownEnd() const;
};
