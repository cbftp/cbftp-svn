#pragma once

#include <string>
#include <ostream>

#include "core/pointer.h"

class SiteLogic;
class FileList;
class Race;
class SiteRace;

class ScoreBoardElement {
  private:
    std::string filename;
    Pointer<SiteLogic> src;
    Pointer<SiteLogic> dst;
    FileList * fls;
    FileList * fld;
    SiteRace * srcsr;
    SiteRace * dstsr;
    Pointer<Race> race;
    unsigned short score;
    bool prio;
    bool attempted;
  public:
    ScoreBoardElement(const std::string &, unsigned short, bool, const Pointer<SiteLogic> &,
        FileList *, SiteRace *, const Pointer<SiteLogic> &, FileList *, SiteRace *, Pointer<Race> &);
    void reset(const std::string &, unsigned short, bool, const Pointer<SiteLogic> &,
        FileList *, SiteRace *, const Pointer<SiteLogic> &, FileList *, SiteRace *, Pointer<Race> &);
    const std::string & fileName() const;
    const Pointer<SiteLogic> & getSource() const;
    const Pointer<SiteLogic> & getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    SiteRace * getSourceSiteRace() const;
    SiteRace * getDestinationSiteRace() const;
    const Pointer<Race> & getRace() const;
    unsigned short getScore() const;
    bool isPrioritized() const;
    bool wasAttempted() const;
    void setAttempted();
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
