#pragma once

#include <string>
#include <map>
#include <list>
#include <vector>

class FileList;

#define RCL_DELETE 0
#define RCL_DELETEOWN 1
#define RCL_TRANSFER 2

#define RCL_ACTION_LIST 1354
#define RCL_ACTION_CWD 1355
#define RCL_ACTION_DELETE 1356

class RecursiveCommandLogic {
private:
  int mode;
  bool active;
  std::string basepath;
  std::string target;
  std::list<std::string> wantedlists;
  std::vector<std::string> deletefiles;
  bool listtarget;
  std::string listtargetpath;
public:
  RecursiveCommandLogic();
  void initialize(int, std::string, std::string);
  bool isActive() const;
  int getAction(std::string, std::string &);
  void addFileList(FileList *);
  void failedCwd();
};

bool lengthSort(std::string, std::string);
