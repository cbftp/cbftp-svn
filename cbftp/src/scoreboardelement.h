#pragma once

#include <string>
#include <ostream>

class SiteLogic;
class FileList;

class ScoreBoardElement {
  private:
    std::string filename;
    SiteLogic * src;
    SiteLogic * dst;
    FileList * fls;
    FileList * fld;
    unsigned short score;
    bool prio;
  public:
    ScoreBoardElement(std::string, unsigned short, bool, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void reset(std::string, unsigned short, bool, SiteLogic *, FileList *, SiteLogic *, FileList *);
    std::string fileName() const;
    SiteLogic * getSource() const;
    SiteLogic * getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    unsigned short getScore() const;
    bool isPrioritized() const;
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
