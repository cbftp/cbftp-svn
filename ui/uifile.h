#pragma once

#include <string>

#include "../file.h"

class UIFile {
private:
  std::string name;
  long int size;
  std::string owner;
  std::string group;
  std::string lastmodified;
  bool directory;
  bool selected;
  bool cursored;
public:
  UIFile(File *);
  bool isDirectory();
  std::string getOwner();
  std::string getGroup();
  long int getSize();
  std::string getLastModified();
  std::string getName();
  bool isSelected();
  bool isCursored();
  void select();
  void unSelect();
  void cursor();
  void unCursor();
};
