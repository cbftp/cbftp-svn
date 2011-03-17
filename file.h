#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

class Site;

class File {
  private:
    std::string name;
    std::string extension;
    long int size;
    std::string owner;
    std::string group;
    std::string lastmodified;
    int updatespeed;
    Site * updatesrc;
    std::string updatedst;
    bool updateflag;
    bool directory;
    int touch;
  public:
    File(std::string, std::string);
    File(std::string, int);
    bool isDirectory();
    std::string getOwner();
    std::string getGroup();
    long int getSize();
    std::string getLastModified();
    std::string getName();
    std::string getExtension();
    Site * getUpdateSrc();
    std::string getUpdateDst();
    int getUpdateSpeed();
    bool updateFlagSet();
    int getCurrentSpeed();
    void setUpdateFlag(Site *, std::string, int);
    void unsetUpdateFlag();
    void setSize(long int);
    void setLastModified(std::string);
    void setOwner(std::string);
    void setGroup(std::string);
    void setTouch(int);
    int getTouch();
};
