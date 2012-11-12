#pragma once

#include <string>
#include <vector>

#include "../globalcontext.h"
#include "../file.h"

#define SIZEPOWER 1024
#define SIZEDECIMALS 2

extern GlobalContext * global;

class UIFile {
private:
  std::string name;
  unsigned long long int size;
  std::string sizerepr;
  std::string owner;
  std::string group;
  int lastmodified;
  std::string lastmodifiedrepr;
  bool directory;
  bool selected;
  bool cursored;
  static unsigned int sizegranularity;
  static std::vector<unsigned long long int> powers;
  void parseTimeStamp(std::string);
  std::string parseSize(unsigned long long int);
public:
  UIFile(File *);
  bool isDirectory();
  std::string getOwner();
  std::string getGroup();
  unsigned long long int getSize();
  std::string getSizeRepr();
  std::string getLastModified();
  int getModifyTime();
  std::string getName();
  static int getSizeGranularity();
  static std::vector<unsigned long long int> getPowers();
  bool isSelected();
  bool isCursored();
  void select();
  void unSelect();
  void cursor();
  void unCursor();
};
