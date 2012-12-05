#pragma once
#include <map>
#include <iostream>
#include <pthread.h>

#include "file.h"
#include "site.h"

class FileList {
  private:
    std::map<std::string, File *> files;
    std::string username;
    std::string path;
    pthread_mutex_t filelist_mutex;
    pthread_mutex_t owned_mutex;
    pthread_mutex_t filled_mutex;
    bool filled;
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
    void setFileUpdateFlag(std::string, unsigned int, Site *, std::string);
    File * getFile(std::string);
    File * getFile(std::string, bool);
    bool isFilled();
    void setFilled();
    std::map<std::string, File *>::iterator begin();
    std::map<std::string, File *>::iterator end();
    bool contains(std::string);
    int getSize();
    int getNumUploadedFiles();
    std::string getPath();
    int getSizeUploaded();
    bool hasSFV();
    std::map<std::string, File *>::iterator filesBegin();
    std::map<std::string, File *>::iterator filesEnd();
    int getOwnedPercentage();
    unsigned long long int getMaxFileSize();
    void lockFileList();
    void unlockFileList();
    void cleanSweep(int);
    void flush();
};
