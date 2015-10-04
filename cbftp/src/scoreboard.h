#pragma once

#include <vector>
#include <string>

#include "pointer.h"

class ScoreBoardElement;
class SiteLogic;
class FileList;
class Race;

#define RESIZE_CHUNK 1000
#define USHORT_MAX 0x10000

class ScoreBoard {
  private:
    std::vector<ScoreBoardElement *> elements;
    std::vector<ScoreBoardElement *> elementstmp;
    std::vector<ScoreBoardElement *> * currelements;
    std::vector<ScoreBoardElement *> * currelementstmp;
    unsigned int showsize;
    unsigned int * count;
    unsigned int * bucketpositions;
    unsigned int countarraybytesize;
  public:
    ScoreBoard();
    ~ScoreBoard();
    void add(std::string, unsigned short, bool, SiteLogic *, FileList *, SiteLogic *, FileList *, Pointer<Race>);
    unsigned int size() const;
    std::vector<ScoreBoardElement *>::const_iterator begin() const;
    std::vector<ScoreBoardElement *>::const_iterator end() const;
    void sort();
    const std::vector<ScoreBoardElement *> * getElementVector() const;
    void wipe();
};
