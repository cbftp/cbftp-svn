#pragma once

#include <string>
#include <map>
#include <list>
#include <vector>

#include "path.h"

class FileList;

#define RCL_DELETE 0
#define RCL_DELETEOWN 1
#define RCL_TRANSFER 2

#define RCL_ACTION_LIST 1354
#define RCL_ACTION_CWD 1355
#define RCL_ACTION_DELETE 1356
#define RCL_ACTION_NOOP 1357

class RecursiveCommandLogic {
private:
  int mode;
  bool active;
  Path basepath;
  Path target;
  std::list<Path> wantedlists;
  std::vector<Path> deletefiles;
  bool listtarget;
  Path listtargetpath;
public:
  RecursiveCommandLogic();
  void initialize(int, const Path &);
  bool isActive() const;
  int getAction(const Path &, Path &);
  void addFileList(FileList *);
  void failedCwd();
};
