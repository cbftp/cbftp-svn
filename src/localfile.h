#pragma once

#include <string>

enum class LocalFileType {
  FILE,
  DIR,
  LINK
};

class LocalFile {
public:
  LocalFile(const std::string& name, unsigned long long int size, LocalFileType type,
      const std::string& owner, const std::string& group, int year, int month,
      int day, int hour, int minute, const std::string& linktarget = "", bool download = false);
  std::string getName() const;
  unsigned long long int getSize() const;
  bool isFile() const;
  bool isDirectory() const;
  bool isLink() const;
  std::string getOwner() const;
  std::string getGroup() const;
  std::string getLinkTarget() const;
  int getYear() const;
  int getMonth() const;
  int getDay() const;
  int getHour() const;
  int getMinute() const;
  void setSize(unsigned long long int size);
  int getTouch() const;
  void setTouch(int touch);
  bool isDownloading() const;
  void setDownloading();
  void finishDownload();
private:
  std::string name;
  unsigned long long int size;
  LocalFileType type;
  std::string owner;
  std::string group;
  std::string linktarget;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int touch;
  bool download;
};
