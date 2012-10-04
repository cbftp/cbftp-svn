#pragma once

#include <string>

#include "../globalcontext.h"
#include "../file.h"

extern GlobalContext * global;

class UIFile {
private:
  std::string name;
  long int size;
  std::string owner;
  std::string group;
  int lastmodified;
  std::string lastmodifiedrepr;
  bool directory;
  bool selected;
  bool cursored;
  void parseTimeStamp(std::string);
public:
  UIFile(File *);
  bool isDirectory();
  std::string getOwner();
  std::string getGroup();
  long int getSize();
  std::string getLastModified();
  int getModifyTime();
  std::string getName();
  bool isSelected();
  bool isCursored();
  void select();
  void unSelect();
  void cursor();
  void unCursor();
};
