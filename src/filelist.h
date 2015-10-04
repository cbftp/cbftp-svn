#pragma once

#include <map>
#include <string>

#define MAXTRANSFERATTEMPTS 5

class File;
class Site;

class FileList {
  private:
    FileList(const FileList &);
    std::map<std::string, File *> files;
    std::string username;
    std::string path;
    bool filled;
    bool locked;
    bool listchanged;
    unsigned long long lastchangedstamp;
    int owned;
    int ownpercentage;
    int uploading;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int uploadedfiles;
    void editOwnedFileCount(bool);
    void setChanged();
  public:
    FileList(const std::string &, const std::string &);
    ~FileList();
    bool updateFile(const std::string &, int);
    void touchFile(const std::string &, const std::string &);
    void touchFile(const std::string &, const std::string &, bool);
    void setFileUpdateFlag(const std::string &, unsigned long long int, unsigned int, Site *, const std::string &);
    File * getFile(const std::string &) const;
    bool isFilled() const;
    void setFilled();
    std::map<std::string, File *>::iterator begin();
    std::map<std::string, File *>::iterator end();
    std::map<std::string, File *>::const_iterator begin() const;
    std::map<std::string, File *>::const_iterator end() const;
    bool contains(const std::string &) const;
    unsigned int getSize() const;
    unsigned long long int getTotalFileSize() const;
    unsigned int getNumUploadedFiles() const;
    std::string getPath() const;
    bool hasSFV() const;
    int getOwnedPercentage() const;
    unsigned long long int getMaxFileSize() const;
    void cleanSweep(int);
    void flush();
    bool listChanged() const;
    void resetListChanged();
    unsigned long long timeSinceLastChanged();
    std::string getUser() const;
    void finishUpload(const std::string &);
    void finishDownload(const std::string &);
    void download(const std::string &);
    bool hasFilesUploading() const;
};
