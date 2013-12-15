#pragma once

#include <string>
#include <vector>
#include <list>

class UIFile;
class FileList;

class UIFileList {
private:
  std::vector<UIFile> files;
  std::vector<UIFile *> sortedfiles;
  std::list<unsigned int> selectedfiles;
  UIFile * current;
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
  UIFile * cursoredFile();
  void selectFileName(std::string);
  bool goNext();
  bool goPrevious();
  void toggleSelectCurrent();
  void unSelect();
  unsigned int size();
  unsigned int sizeFiles();
  unsigned int sizeDirs();
  unsigned long long int getTotalSize();
  std::vector <UIFile *> * getSortedList();
  unsigned int currentCursorPosition();
  std::string getPath();
  std::string getSortMethod();
  void removeFile(std::string);
  void toggleSeparators();
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
