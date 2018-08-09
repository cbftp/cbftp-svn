#pragma once

#include <memory>
#include <ostream>
#include <string>


class SiteLogic;
class FileList;
class Race;
class SiteRace;

class ScoreBoardElement {
  private:
    std::string filename;
    std::shared_ptr<SiteLogic> src;
    std::shared_ptr<SiteLogic> dst;
    FileList * fls;
    FileList * fld;
    SiteRace * srcsr;
    SiteRace * dstsr;
    std::shared_ptr<Race> race;
    unsigned short score;
    bool prio;
    bool attempted;
    std::string subdir;
  public:
    ScoreBoardElement(const std::string &, unsigned short, bool, const std::shared_ptr<SiteLogic> &,
        FileList *, SiteRace *, const std::shared_ptr<SiteLogic> &, FileList *, SiteRace *, std::shared_ptr<Race> &, const std::string &);
    void reset(const std::string &, unsigned short, bool, const std::shared_ptr<SiteLogic> &,
        FileList *, SiteRace *, const std::shared_ptr<SiteLogic> &, FileList *, SiteRace *, std::shared_ptr<Race> &, const std::string &);
    const std::string & fileName() const;
    const std::string & subDir() const;
    const std::shared_ptr<SiteLogic> & getSource() const;
    const std::shared_ptr<SiteLogic> & getDestination() const;
    FileList * getSourceFileList() const;
    FileList * getDestinationFileList() const;
    SiteRace * getSourceSiteRace() const;
    SiteRace * getDestinationSiteRace() const;
    const std::shared_ptr<Race> & getRace() const;
    unsigned short getScore() const;
    bool isPrioritized() const;
    bool wasAttempted() const;
    void setAttempted();
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
