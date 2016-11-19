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
    std::list<FileList *> sizeestimated;
    std::map<FileList *, unsigned long long int> observestarts;
    std::map<FileList *, unsigned long long int> sfvobservestarts;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int numuploadedfiles;
    int profile;
    void updateNumFilesUploaded();
    void addNewDirectories();
  public:
    int classType() const;
    std::string getSiteName() const;
    const Path & getSection() const;
    std::string getRelease() const;
    std::string getGroup() const;
    const Path & getPath() const;
    unsigned int getId() const;
    std::string getRelevantSubPath();
    SiteRace(Pointer<Race>, const std::string &, const Path &, const std::string &, const std::string &);
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
    bool isGlobalDone() const;
    int getProfile() const;
    void complete(bool);
    void abort();
    void reset();
    void subPathComplete(FileList *);
    bool isSubPathComplete(const std::string &) const;
    bool isSubPathComplete(FileList *) const;
    void reportSize(FileList *, const std::set<std::string> &, bool);
    int getObservedTime(FileList *);
    int getSFVObservedTime(FileList *);
    bool hasBeenUpdatedSinceLastCheck();
};
