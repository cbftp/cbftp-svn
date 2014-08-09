#pragma once

#include <string>

class SiteLogic;
class FileList;

class ScoreBoardElement {
  private:
    std::string filename;
    SiteLogic * src;
    SiteLogic * dst;
    FileList * fls;
    FileList * fld;
    int score;
    bool prio;
  public:
    ScoreBoardElement(std::string, int, bool, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void reset(std::string, int, bool, SiteLogic *, FileList *, SiteLogic *, FileList *);
    std::string fileName() const;
    SiteLogic * getSource() const;
    SiteLogic * getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    int getScore() const;
    bool isPrioritized() const;
};
