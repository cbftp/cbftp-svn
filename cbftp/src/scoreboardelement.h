#pragma once

#include <string>
#include <ostream>

#include "pointer.h"

class SiteLogic;
class FileList;
class Race;

class ScoreBoardElement {
  private:
    std::string filename;
    SiteLogic * src;
    SiteLogic * dst;
    FileList * fls;
    FileList * fld;
    Pointer<Race> race;
    unsigned short score;
    bool prio;
  public:
    ScoreBoardElement(std::string, unsigned short, bool, SiteLogic *, FileList *, SiteLogic *, FileList *, Pointer<Race>);
    void reset(std::string, unsigned short, bool, SiteLogic *, FileList *, SiteLogic *, FileList *, Pointer<Race>);
    std::string fileName() const;
    SiteLogic * getSource() const;
    SiteLogic * getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    Pointer<Race> getRace() const;
    unsigned short getScore() const;
    bool isPrioritized() const;
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
