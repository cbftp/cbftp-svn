#include "uifilelist.h"

UIFileList::UIFileList() {
  currentposition = 0;
  path = "";
}

void UIFileList::sortName(bool ascending) {

}

void UIFileList::sortTime(bool ascending) {

}

void UIFileList::parse(FileList * filelist) {
  files.clear();
  sortedfiles.clear();
  selectedfiles.clear();
  currentposition = 0;
  std::map<std::string, File *>::iterator it;
  int size = filelist->getSize();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = filelist->filesBegin(); it != filelist->filesEnd(); it++) {
    files.push_back(UIFile(it->second));
  }
  for (unsigned int i = 0; i < files.size(); i++) {
    sortedfiles.push_back(&(files[i]));
  }
  path = filelist->getPath();
}

UIFile * UIFileList::cursoredFile() {
  return sortedfiles[currentposition];
}

bool UIFileList::goNext() {
  if (currentposition < size() - 1) {
    currentposition++;
    return true;
  }
  return false;
}

bool UIFileList::goPrevious() {
  if (currentposition > 0) {
    currentposition--;
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
