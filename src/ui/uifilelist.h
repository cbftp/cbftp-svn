#pragma once

#include <string>
#include <vector>
#include <list>

#include "uifile.h"

#include "../pointer.h"

class FileList;
class LocalFileList;

class UIFileList {
private:
  std::vector<UIFile> files;
  std::vector<UIFile *> sortedfiles;
  std::list<unsigned int> selectedfiles;
  unsigned int currentposition;
  UIFile * currentcursored;
  std::string path;
  unsigned int numfiles;
  unsigned int numdirs;
  unsigned long long int totalsize;
  std::string sortmethod;
  bool separators;
  void setNewCurrentPosition();
  void removeSeparators();
public:
  UIFileList();
  void sortCombined();
  void sortName(bool);
  void sortTime(bool);
  void sortSize(bool);
  void sortOwner(bool);
  void parse(FileList *);
  void parse(Pointer<LocalFileList> &);
  UIFile * cursoredFile() const;
  void selectFileName(std::string);
  bool goNext();
  bool goPrevious();
  void toggleSelectCurrent();
  void unSelect();
  unsigned int size() const;
  unsigned int sizeFiles() const;
  unsigned int sizeDirs() const;
  unsigned long long int getTotalSize() const;
  const std::vector <UIFile *> * getSortedList() const;
  unsigned int currentCursorPosition() const;
  std::string getPath() const;
  std::string getSortMethod() const;
  bool separatorsEnabled() const;
  void removeFile(std::string);
  void toggleSeparators();
  void setCursorPosition(unsigned int);
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
