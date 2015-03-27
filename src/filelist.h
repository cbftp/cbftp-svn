#pragma once

#include <map>
#include <string>

#define MAXTRANSFERATTEMPTS 5

class File;
class Site;

class FileList {
  private:
    std::map<std::string, File *> files;
    std::map<std::string, int> downloadattempts;
    std::map<std::string, int> uploadattempts;
    std::string username;
    std::string path;
    bool filled;
    bool locked;
    bool listchanged;
    unsigned long long lastchangedstamp;
    int owned;
    int ownpercentage;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int uploadedfiles;
    void editOwnedFileCount(bool);
    void setChanged();
  public:
    FileList(std::string, std::string);
    ~FileList();
    bool updateFile(std::string, int);
    void touchFile(std::string, std::string);
    void touchFile(std::string, std::string, bool);
    void setFileUpdateFlag(std::string, unsigned long long int, unsigned int, Site *, std::string);
    File * getFile(std::string) const;
    bool isFilled() const;
    void setFilled();
    std::map<std::string, File *>::iterator begin();
    std::map<std::string, File *>::iterator end();
    std::map<std::string, File *>::const_iterator begin() const;
    std::map<std::string, File *>::const_iterator end() const;
    bool contains(std::string) const;
    unsigned int getSize() const;
    unsigned long long int getTotalFileSize() const;
    unsigned int getNumUploadedFiles() const;
    std::string getPath() const;
    bool hasSFV() const;
    int getOwnedPercentage() const;
    unsigned long long int getMaxFileSize() const;
    void cleanSweep(int);
    void flush();
    void uploadFail(std::string);
    void downloadFail(std::string);
    void addUploadAttempt(std::string);
    void downloadAttemptFail(std::string);
    bool hasFailedDownload(std::string) const;
    bool hasFailedUpload(std::string) const;
    bool listChanged() const;
    void resetListChanged();
    unsigned long long timeSinceLastChanged();
    std::string getUser() const;
};
