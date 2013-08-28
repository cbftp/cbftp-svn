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
  std::string linktarget;
  int lastmodified;
  std::string lastmodifiedrepr;
  bool directory;
  bool softlink;
  bool selected;
  bool cursored;
  static unsigned int sizegranularity;
  static std::vector<unsigned long long int> powers;
  void parseTimeStamp(std::string);
public:
  UIFile(File *);
  bool isDirectory();
  bool isLink();
  std::string getOwner();
  std::string getGroup();
  unsigned long long int getSize();
  std::string getSizeRepr();
  std::string getLastModified();
  int getModifyTime();
  std::string getName();
  std::string getLinkTarget();
  static int getSizeGranularity();
  static std::vector<unsigned long long int> getPowers();
  static std::string parseSize(unsigned long long int);
  bool isSelected();
  bool isCursored();
  void select();
  void unSelect();
  void cursor();
  void unCursor();
};
