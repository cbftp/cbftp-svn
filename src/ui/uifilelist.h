#pragma once

#include <vector>
#include <list>
#include <algorithm>

#include "../filelist.h"
#include "../file.h"

#include "uifile.h"

class UIFileList {
private:
  std::vector<UIFile> files;
  std::vector<UIFile *> sortedfiles;
  std::list<unsigned int> selectedfiles;
  UIFile * current;
  unsigned int currentposition;
  UIFile * currentcursored;
  std::string path;
  void setNewCurrentPosition();
public:
  UIFileList();
  void sortCombined();
  void sortName(bool);
  void sortTime(bool);
  void sortSize(bool);
  void sortOwner(bool);
  void parse(FileList *);
  UIFile * cursoredFile();
  void selectFileName(std::string);
  bool goNext();
  bool goPrevious();
  void toggleSelectCurrent();
  void unSelect();
  unsigned int size();
  std::vector <UIFile *> * getSortedList();
  unsigned int currentCursorPosition();
  std::string getPath();
};

bool combinedSort(UIFile *, UIFile *);
bool nameSortAsc(UIFile *, UIFile *);
bool nameSortDesc(UIFile *, UIFile *);
bool timeSortAsc(UIFile *, UIFile *);
bool timeSortDesc(UIFile *, UIFile *);
bool sizeSortAsc(UIFile *, UIFile *);
bool sizeSortDesc(UIFile *, UIFile *);
bool ownerSortAsc(UIFile *, UIFile *);
bool ownerSortDesc(UIFile *, UIFile *);
