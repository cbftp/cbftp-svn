#pragma once

#include <string>

#include "../timestamps.h"

class File;
class LocalFile;

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
  bool softselected;
  bool hardselected;
  void setLastModified(const timestamps::Timestamp& timestamp);
public:
  UIFile(File *);
  UIFile(const LocalFile &);
  bool isDirectory() const;
  bool isLink() const;
  std::string getOwner() const;
  std::string getGroup() const;
  unsigned long long int getSize() const;
  std::string getSizeRepr() const;
  std::string getLastModified() const;
  int getModifyTime() const;
  int getModifyDate() const;
  std::string getName() const;
  std::string getLinkTarget() const;
  bool isSoftSelected() const;
  bool isHardSelected() const;
  void softSelect();
  void hardSelect();
  void unSoftSelect();
  void unHardSelect();
};
