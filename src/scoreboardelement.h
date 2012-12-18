#pragma once

#include <string>

class SiteThread;
class FileList;

class ScoreBoardElement {
  private:
    std::string filename;
    SiteThread * src;
    SiteThread * dst;
    FileList * fls;
    FileList * fld;
    int score;
  public:
    ScoreBoardElement(std::string, int, SiteThread *, FileList *, SiteThread *, FileList *);
    std::string fileName();
    SiteThread * getSource();
    SiteThread * getDestination();
    FileList * getSourceFileList();
    FileList * getDestinationFileList();
    int getScore();
};
