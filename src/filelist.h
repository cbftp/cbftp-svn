#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "path.h"

enum FileListState {
 FILELIST_UNKNOWN,
 FILELIST_NONEXISTENT,
 FILELIST_EXISTS,
 FILELIST_LISTED,
 FILELIST_FAILED
};

class File;
class Site;
class CommandOwner;

class FileList {
  private:
    FileList(const FileList &);
    File * getFileCaseSensitive(const std::string &) const;
    std::unordered_map<std::string, File *> files;
    std::unordered_map<std::string, std::string> lowercasefilemap;
    std::string username;
    Path path;
    FileListState state;
    bool locked;
    bool listchanged;
    bool listmetachanged;
    unsigned long long lastchangedstamp;
    unsigned long long lastmetachangedstamp;
    int owned;
    int ownpercentage;
    int uploading;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int uploadedfiles;
    int refreshedtime;
    int listfailures;
    void recalcOwnedPercentage();
    void init(const std::string &, const Path &, FileListState);
    void setChanged();
    void setMetaChanged();
  public:
    FileList(const std::string &, const Path &);
    FileList(const std::string &, const Path &, FileListState);
    ~FileList();
    bool updateFile(const std::string &, int);
    void touchFile(const std::string &, const std::string &);
    void touchFile(const std::string &, const std::string &, bool);
    void removeFile(const std::string &);
    void setFileUpdateFlag(const std::string &, unsigned long long int, unsigned int, const std::shared_ptr<Site> &, const std::shared_ptr<Site> &, CommandOwner *, CommandOwner *);
    File * getFile(const std::string &) const;
    FileListState getState() const;
    void setNonExistent();
    void setExists();
    void setFilled();
    void setFailed();
    std::unordered_map<std::string, File *>::iterator begin();
    std::unordered_map<std::string, File *>::iterator end();
    std::unordered_map<std::string, File *>::const_iterator begin() const;
    std::unordered_map<std::string, File *>::const_iterator end() const;
    bool contains(const std::string &) const;
    bool containsPattern(const std::string &, bool) const;
    bool containsPatternBefore(const std::string &, bool, const std::string &) const;
    unsigned int getSize() const;
    unsigned long long int getTotalFileSize() const;
    unsigned int getNumUploadedFiles() const;
    const Path & getPath() const;
    bool hasSFV() const;
    int getOwnedPercentage() const;
    unsigned long long int getMaxFileSize() const;
    void cleanSweep(int);
    void flush();
    bool listChanged() const;
    bool listMetaChanged() const;
    void resetListChanged();
    unsigned long long timeSinceLastChanged() const;
    unsigned long long timeSinceLastMetaChanged() const;
    std::string getUser() const;
    void finishUpload(const std::string &);
    void finishDownload(const std::string &);
    void download(const std::string &);
    bool hasFilesUploading() const;
    void setRefreshedTime(int);
    int getRefreshedTime() const;
    bool addListFailure();
};
