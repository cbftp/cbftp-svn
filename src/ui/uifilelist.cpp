#include "uifilelist.h"

UIFileList::UIFileList() {
  currentposition = 0;
  currentcursored = NULL;
  path = "";
}

bool combinedSort(UIFile * a, UIFile * b) {
  bool aisdir = a->isDirectory();
  bool bisdir = b->isDirectory();
  if (aisdir && !bisdir) return true;
  if (!aisdir && bisdir) return false;
  if (aisdir && bisdir) return timeSortDesc(a, b);
  return sizeSortDesc(a, b);
}

bool nameSortAsc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
  return a->getName().compare(b->getName()) < 0;
}

bool nameSortDesc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
  return a->getName().compare(b->getName()) > 0;
}

bool timeSortAsc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool timeSortDesc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortDesc(a, b);
  return diff > 0;
}

bool sizeSortAsc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
  long int diff = a->getSize() - b->getSize();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool sizeSortDesc(UIFile * a, UIFile * b) {
  if (a->isDirectory() && !b->isDirectory()) return true;
  if (!a->isDirectory() && b->isDirectory()) return false;
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
  std::sort(sortedfiles.begin(), sortedfiles.end(), combinedSort);
  setNewCurrentPosition();
}
void UIFileList::sortName(bool ascending) {
  if (ascending) std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortAsc);
  else std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortDesc);
  setNewCurrentPosition();
}

void UIFileList::sortTime(bool ascending) {
  if (ascending) std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortAsc);
  else std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortDesc);
  setNewCurrentPosition();
}

void UIFileList::sortSize(bool ascending) {
  if (ascending) std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortAsc);
  else std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortDesc);
  setNewCurrentPosition();
}

void UIFileList::sortOwner(bool ascending) {
  if (ascending) std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortAsc);
  else std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortDesc);
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
  else {
    if (sortedfiles.size() > 0) {
      currentcursored = sortedfiles[0];
    }
  }
}

void UIFileList::selectFileName(std::string filename) {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i]->getName() == filename) {
      currentposition = i;
      currentcursored = sortedfiles[i];
      return;
    }
  }
}

void UIFileList::parse(FileList * filelist) {
  files.clear();
  sortedfiles.clear();
  selectedfiles.clear();
  currentposition = 0;
  currentcursored = NULL;
  std::map<std::string, File *>::iterator it;
  int size = filelist->getSize();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = filelist->begin(); it != filelist->end(); it++) {
    files.push_back(UIFile(it->second));
  }
  for (unsigned int i = 0; i < files.size(); i++) {
    sortedfiles.push_back(&(files[i]));
  }
  path = filelist->getPath();
}

UIFile * UIFileList::cursoredFile() {
  return currentcursored;
}

bool UIFileList::goNext() {
  if (size() > 0 && currentposition < size() - 1) {
    currentposition++;
    currentcursored = sortedfiles[currentposition];
    return true;
  }
  return false;
}

bool UIFileList::goPrevious() {
  if (size() > 0 && currentposition > 0) {
    currentposition--;
    currentcursored = sortedfiles[currentposition];
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

unsigned int UIFileList::size() {
  return files.size();
}

std::vector<UIFile *> * UIFileList::getSortedList() {
  return &sortedfiles;
}

unsigned int UIFileList::currentCursorPosition() {
  return currentposition;
}

std::string UIFileList::getPath() {
  return path;
}
