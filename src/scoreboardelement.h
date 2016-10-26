#pragma once

#include <string>
#include <ostream>

#include "core/pointer.h"

class SiteLogic;
class FileList;
class Race;

class ScoreBoardElement {
  private:
    std::string filename;
    Pointer<SiteLogic> src;
    Pointer<SiteLogic> dst;
    FileList * fls;
    FileList * fld;
    Pointer<Race> race;
    unsigned short score;
    bool prio;
  public:
    ScoreBoardElement(const std::string &, unsigned short, bool, const Pointer<SiteLogic> &, FileList *, const Pointer<SiteLogic> &, FileList *, Pointer<Race> &);
    void reset(const std::string &, unsigned short, bool, const Pointer<SiteLogic> &, FileList *, const Pointer<SiteLogic> &, FileList *, Pointer<Race> &);
    const std::string & fileName() const;
    const Pointer<SiteLogic> & getSource() const;
    const Pointer<SiteLogic> & getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    const Pointer<Race> & getRace() const;
    unsigned short getScore() const;
    bool isPrioritized() const;
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
