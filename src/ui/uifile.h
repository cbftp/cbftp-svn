#pragma once

#include <string>

class File;

class UIFile {
private:
  std::string name;
  unsigned long long int size;
  std::string sizerepr;
  std::string owner;
  std::string group;
  std::string linktarget;
  int lastmodified;
  int lastmodifieddate;
  std::string lastmodifiedrepr;
  bool directory;
  bool softlink;
  bool selected;
  bool cursored;
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
  int getModifyDate();
  std::string getName();
  std::string getLinkTarget();
  bool isSelected();
  bool isCursored();
  void select();
  void unSelect();
  void cursor();
  void unCursor();
};
