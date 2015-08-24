#pragma once

#include <string>

class Site;

class File {
  friend class FileList;
private:
  void parseUNIXSTATLine(const std::string &);
  void parseBrokenUNIXSTATLine(const std::string &, int, int &);
  void parseMSDOSSTATLine(const std::string &);
  std::string digitsOnly(const std::string &) const;
  std::string name;
  std::string linktarget;
  std::string extension;
  unsigned long long int size;
  std::string owner;
  std::string group;
  std::string lastmodified;
  unsigned int updatespeed;
  Site * updatesrc;
  std::string updatedst;
  bool updateflag;
  bool directory;
  bool softlink;
  int touch;
  bool uploading;
  int downloading;
protected:
  File(const std::string &, const std::string &);
  File(const std::string &, int);
  void setUpdateFlag(Site *, std::string, int);
  void unsetUpdateFlag();
  bool setSize(unsigned long long int);
  bool setLastModified(const std::string &);
  bool setOwner(const std::string &);
  bool setGroup(const std::string &);
  void setTouch(int);
  void download();
  void upload();
  void finishUpload();
  void finishDownload();
public:
  static std::string getExtension(const std::string &);
  bool isDirectory() const;
  bool isLink() const;
  std::string getOwner() const;
  std::string getGroup() const;
  unsigned long long int getSize() const;
  std::string getLastModified() const;
  std::string getName() const;
  std::string getLinkTarget() const;
  std::string getExtension() const;
  Site * getUpdateSrc() const;
  std::string getUpdateDst() const;
  unsigned int getUpdateSpeed() const;
  bool updateFlagSet() const;
  bool isDownloading() const;
  bool isUploading() const;
  int getTouch() const;
};
