#pragma once

#include <vector>
#include <list>

#include "filelist.h"
#include "file.h"
#include "uifile.h"

class UIFileList {
private:
  std::vector<UIFile> files;
  std::vector<UIFile *> sortedfiles;
  std::list<unsigned int> selectedfiles;
  UIFile * current;
  unsigned int currentposition;
  std::string path;
public:
  UIFileList();
  void sortName(bool);
  void sortTime(bool);
  void parse(FileList *);
  UIFile * cursoredFile();
  bool goNext();
  bool goPrevious();
  void toggleSelectCurrent();
  void unSelect();
  unsigned int size();
  std::vector <UIFile *> * getSortedList();
  unsigned int currentCursorPosition();
  std::string getPath();
};
