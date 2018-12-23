#pragma once

#include <memory>
#include <ostream>
#include <string>


class SiteLogic;
class FileList;
class Race;
class SiteRace;

enum class PrioType {
  PRIO,
  PRIO_LATER,
  NORMAL
};

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
    PrioType priotype;
    bool attempted;
    std::string subdir;
    unsigned long long int filesize;
  public:
    ScoreBoardElement(const std::string &, unsigned short, unsigned long long int filesize, PrioType priotype, const std::shared_ptr<SiteLogic> &,
        FileList *, SiteRace *, const std::shared_ptr<SiteLogic> &, FileList *, SiteRace *, const std::shared_ptr<Race> &, const std::string &);
    void reset(const std::string &, unsigned short, unsigned long long int filesize, PrioType priotype, const std::shared_ptr<SiteLogic> &,
        FileList *, SiteRace *, const std::shared_ptr<SiteLogic> &, FileList *, SiteRace *, const std::shared_ptr<Race> &, const std::string &);
    void reset(const ScoreBoardElement & other);
    void update(unsigned short);
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
    PrioType getPriorityType() const;
    unsigned long long int getFileSize() const;
    bool wasAttempted() const;
    void setAttempted();
};

std::ostream & operator<<(std::ostream &, const ScoreBoardElement &);
