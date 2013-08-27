#pragma once
#include <map>
#include <iostream>
#include <pthread.h>

#include "file.h"
#include "site.h"

class FileList {
  private:
    std::map<std::string, File *> files;
    std::map<std::string, int> downloadfails;
    std::map<std::string, int> uploadfails;
    std::string username;
    std::string path;
    pthread_mutex_t filelist_mutex;
    pthread_mutex_t owned_mutex;
    pthread_mutex_t filled_mutex;
    bool filled;
    bool locked;
    int owned;
    int ownpercentage;
    unsigned long long int maxfilesize;
    int uploadedfiles;
    File * getFileIntern(std::string);
    void editOwnedFileCount(bool);
  public:
    FileList(std::string, std::string);
    bool updateFile(std::string, int);
    void touchFile(std::string, std::string);
    void touchFile(std::string, std::string, bool);
    void setFileUpdateFlag(std::string, long int, unsigned int, Site *, std::string);
    File * getFile(std::string);
    File * getFile(std::string, bool);
    bool isFilled();
    void setFilled();
    std::map<std::string, File *>::iterator begin();
    std::map<std::string, File *>::iterator end();
    bool contains(std::string);
    unsigned int getSize();
    unsigned int getNumUploadedFiles();
    std::string getPath();
    int getSizeUploaded();
    bool hasSFV();
    int getOwnedPercentage();
    unsigned long long int getMaxFileSize();
    void lockFileList();
    void unlockFileList();
    void cleanSweep(int);
    void flush();
    void uploadFail(std::string);
    void downloadFail(std::string);
    void uploadAttemptFail(std::string);
    void downloadAttemptFail(std::string);
    bool hasFailedDownload(std::string);
    bool hasFailedUpload(std::string);
};
