#include "uifilelist.h"

#include <algorithm>

#include "../filelist.h"
#include "../file.h"
#include "../localfilelist.h"
#include "../localfile.h"
#include "../util.h"

UIFileList::UIFileList() :
  currentposition(0),
  currentcursored(NULL),
  numfiles(0),
  numdirs(0),
  totalsize(0),
  filterednumfiles(0),
  filterednumdirs(0),
  filteredtotalsize(0),
  sortmethod("none"),
  separators(false)
{
}

bool combinedSort(UIFile * a, UIFile * b) {
  bool aisdir = a->isDirectory();
  bool bisdir = b->isDirectory();
  bool aislink = a->isLink();
  bool bislink = b->isLink();
  if ((aislink || aisdir) && !bislink && !bisdir) return true;
  if (!aislink && !aisdir && (bislink || bisdir)) return false;
  if ((aisdir || aislink) && (bisdir || bislink)) return timeSortDesc(a, b);
  return sizeSortDesc(a, b);
}

bool nameSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  return a->getName().compare(b->getName()) < 0;
}

bool nameSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  return a->getName().compare(b->getName()) > 0;
}

bool timeSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool timeSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortDesc(a, b);
  return diff > 0;
}

bool sizeSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  long int diff = a->getSize() - b->getSize();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool sizeSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  long int diff = a->getSize() - b->getSize();
  if (diff == 0) return nameSortAsc(a, b);
  return diff > 0;
}

bool ownerSortAsc(UIFile * a, UIFile * b) {
  int diff = a->getOwner().compare(b->getOwner());
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool ownerSortDesc(UIFile * a, UIFile * b) {
  int diff = a->getOwner().compare(b->getOwner());
  if (diff == 0) return nameSortAsc(a, b);
  return diff > 0;
}
void UIFileList::sortCombined() {
  removeSeparators();
  std::sort(sortedfiles.begin(), sortedfiles.end(), combinedSort);
  if (separators) {
    int lastdate = 0;
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL && (*it)->getModifyDate() != lastdate) {
          lastdate = (*it)->getModifyDate();
          if (*lastit != NULL && (it != sortedfiles.begin() || !(*it)->isDirectory())) {
            sortedfiles.insert(it, NULL);
            complete = false;
            break;
          }
        }
        lastit = it;
      }
    }
  }
  setNewCurrentPosition();
  sortmethod = "Combined";
}

void UIFileList::sortName(bool ascending) {
  removeSeparators();
  if (ascending) {
    std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortAsc);
    sortmethod = "Name (ascending)";
  }
  else {
    std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortDesc);
    sortmethod = "Name (descending)";
  }
  if (separators) {
    std::string lastletter = "";
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL) {
          std::string firstchar = (*it)->getName().substr(0, 1);
          if (firstchar != lastletter) {
            lastletter = firstchar;
            if (*lastit != NULL && it != sortedfiles.begin()) {
              sortedfiles.insert(it, NULL);
              complete = false;
              break;
            }
          }
        }
        lastit = it;
      }
    }
  }
  setNewCurrentPosition();
}

void UIFileList::sortTime(bool ascending) {
  removeSeparators();
  if (ascending) {
    std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortAsc);
    sortmethod = "Time (ascending)";
  }
  else {
    std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortDesc);
    sortmethod = "Time (descending)";
  }
  if (separators) {
    int lastdate = 0;
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL && (*it)->getModifyDate() != lastdate) {
          lastdate = (*it)->getModifyDate();
          if (*lastit != NULL && it != sortedfiles.begin()) {
            sortedfiles.insert(it, NULL);
            complete = false;
            break;
          }
        }
        lastit = it;
      }
    }
  }
  setNewCurrentPosition();
}

void UIFileList::sortSize(bool ascending) {
  removeSeparators();
  if (ascending) {
    std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortAsc);
    sortmethod = "Size (ascending)";
  }
  else {
    std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortDesc);
    sortmethod = "Size (descending)";
  }
  setNewCurrentPosition();
}

void UIFileList::sortOwner(bool ascending) {
  removeSeparators();
  if (ascending) {
    std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortAsc);
    sortmethod = "Owner (ascending)";
  }
  else {
    std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortDesc);
    sortmethod = "Owner (descending)";
  }
  if (separators) {
    std::string lastowner;
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL && (*it)->getOwner() != lastowner) {
          lastowner = (*it)->getOwner();
          if (*lastit != NULL && it != sortedfiles.begin()) {
            sortedfiles.insert(it, NULL);
            complete = false;
            break;
          }
        }
        lastit = it;
      }
    }
  }
  setNewCurrentPosition();
}

void UIFileList::setNewCurrentPosition() {
  if (currentcursored != NULL) {
    for (unsigned int i = 0; i < sortedfiles.size(); i++) {
      if (sortedfiles[i] == currentcursored) {
        currentposition = i;
        return;
      }
    }
  }
  currentposition = 0;
  currentcursored = NULL;
  if (sortedfiles.size() > 0) {
    currentcursored = sortedfiles[currentposition];
  }
}

void UIFileList::selectFileName(std::string filename) {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i] != NULL && sortedfiles[i]->getName() == filename) {
      currentposition = i;
      currentcursored = sortedfiles[i];
      return;
    }
  }
}

void UIFileList::fillSortedFiles() {
  sortedfiles.clear();
  filterednumfiles = 0;
  filterednumdirs = 0;
  filteredtotalsize = 0;
  for (unsigned int i = 0; i < files.size(); i++) {
    UIFile & file = files[i];
    bool negativepass = true;
    bool foundpositive = false;
    bool positivematch = false;
    for (std::list<std::string>::const_iterator it = filters.begin(); it != filters.end(); it++) {
      const std::string & filter = *it;
      if (filter[0] == '!') {
        if (util::wildcmp(filter.substr(1).c_str(), file.getName().c_str())) {
          negativepass = false;
          break;
        }
      }
      else {
        foundpositive = true;
        if (util::wildcmp(filter.c_str(), file.getName().c_str())) {
          positivematch = true;
        }
      }
    }
    if (!negativepass || (foundpositive && !positivematch)) {
      continue;
    }
    sortedfiles.push_back(&file);
    if (file.isDirectory()) {
      filterednumdirs++;
    }
    else {
      filterednumfiles++;
    }
    filteredtotalsize += file.getSize();
  }
}

void UIFileList::parse(FileList * filelist) {
  files.clear();
  numfiles = 0;
  numdirs = 0;
  totalsize = 0;
  selectedfiles.clear();
  currentposition = 0;
  currentcursored = NULL;
  separators = false;
  filters.clear();
  std::map<std::string, File *>::iterator it;
  int size = filelist->getSize();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = filelist->begin(); it != filelist->end(); it++) {
    files.push_back(UIFile(it->second));
    totalsize += it->second->getSize();
    if (it->second->isDirectory()) {
      numdirs++;
    }
    else {
      numfiles++;
    }
  }
  fillSortedFiles();
  path = filelist->getPath();
}

void UIFileList::parse(Pointer<LocalFileList> & filelist) {
  files.clear();
  numfiles = 0;
  numdirs = 0;
  totalsize = 0;
  selectedfiles.clear();
  currentposition = 0;
  currentcursored = NULL;
  separators = false;
  filters.clear();
  std::map<std::string, LocalFile>::const_iterator it;
  int size = filelist->size();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = filelist->begin(); it != filelist->end(); it++) {
    const LocalFile & f = it->second;
    files.push_back(UIFile(f));
    totalsize += f.getSize();
    if (f.isDirectory()) {
      numdirs++;
    }
    else {
      numfiles++;
    }
  }
  fillSortedFiles();
  path = filelist->getPath();
}

UIFile * UIFileList::cursoredFile() const {
  return currentcursored;
}

bool UIFileList::goNext() {
  if (size() > 0 && currentposition < size() - 1) {
    while ((currentcursored = sortedfiles[++currentposition]) == NULL) {
      if (currentposition >= size() - 1) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool UIFileList::goPrevious() {
  if (size() > 0 && currentposition > 0) {
    while ((currentcursored = sortedfiles[--currentposition]) == NULL) {
      if (!currentposition) {
        return false;
      }
    }
    return true;
  }
  return false;
}

void UIFileList::toggleSelectCurrent() {
  std::list<unsigned int>::iterator it;
  for (it = selectedfiles.begin(); it != selectedfiles.end(); it++) {
    if (*it == currentposition) {
      selectedfiles.erase(it);
      return;
    }
  }
  selectedfiles.push_back(currentposition);
}

void UIFileList::unSelect() {
  selectedfiles.clear();
}

unsigned int UIFileList::size() const {
  return sortedfiles.size();
}

unsigned int UIFileList::sizeFiles() const {
  return numfiles;
}

unsigned int UIFileList::sizeDirs() const {
  return numdirs;
}

unsigned long long int UIFileList::getTotalSize() const {
  return totalsize;
}

unsigned int UIFileList::filteredSizeFiles() const {
  return filterednumfiles;
}

unsigned int UIFileList::filteredSizeDirs() const {
  return filterednumdirs;
}

unsigned long long int UIFileList::getFilteredTotalSize() const {
  return filteredtotalsize;
}

const std::vector<UIFile *> * UIFileList::getSortedList() const {
  return &sortedfiles;
}

unsigned int UIFileList::currentCursorPosition() const {
  return currentposition;
}

std::string UIFileList::getPath() const {
  return path;
}

std::string UIFileList::getSortMethod() const {
  return sortmethod;
}

bool UIFileList::separatorsEnabled() const {
  return separators;
}

void UIFileList::removeFile(std::string file) {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i] != NULL && sortedfiles[i]->getName() == file) {
      totalsize -= sortedfiles[i]->getSize();
      if (sortedfiles[i]->isDirectory()) {
        numdirs--;
      }
      else {
        numfiles--;
      }
      if (currentcursored == sortedfiles[i]) {
        if (sortedfiles.size() > i + 1) {
          if (!goNext()) {
            currentcursored = NULL;
          }
        }
        else if (sortedfiles.size() == 1) {
          currentcursored = NULL;
        }
        else {
          if (!goPrevious()) {
            currentcursored = NULL;
          }
        }
      }
      sortedfiles.erase(sortedfiles.begin() + i);
      setNewCurrentPosition();
      break;
    }
  }
}

void UIFileList::toggleSeparators() {
  if (!separators) {
    separators = true;
  }
  else {
    separators = false;
  }
}

void UIFileList::removeSeparators() {
  bool complete = false;
  while(!complete) {
    complete = true;
    for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
      if (*it == NULL) {
        sortedfiles.erase(it);
        complete = false;
        break;
      }
    }
  }
}

bool UIFileList::hasFilters() const {
  return filters.size();
}

std::list<std::string> UIFileList::getFilters() const {
  return filters;
}

void UIFileList::setFilters(const std::list<std::string> & filters) {
  this->filters = filters;
  fillSortedFiles();
}

void UIFileList::unsetFilters() {
  filters.clear();
  fillSortedFiles();
}

void UIFileList::setCursorPosition(unsigned int position) {
  if (sortedfiles.size()) {
    while (position >= sortedfiles.size()) {
      position--;
    }
    currentposition = position;
    currentcursored = sortedfiles[currentposition];
  }
  else {
    currentposition = 0;
    currentcursored = NULL;
  }
}
