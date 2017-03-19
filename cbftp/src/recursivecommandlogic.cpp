#include "recursivecommandlogic.h"

#include <algorithm>

#include "filelist.h"
#include "file.h"

bool lengthSort(const std::pair<Path, bool> & a, const std::pair<Path, bool> & b) {
  if (a.first.toString().length() < b.first.toString().length()) {
    return true;
  }
  return false;
}

RecursiveCommandLogic::RecursiveCommandLogic() {
  active = false;
}

void RecursiveCommandLogic::initialize(RecursiveCommandType mode, const Path & target) {
  this->mode = mode;
  active = true;
  listtarget = false;
  this->basepath = target.dirName();
  this->target = target;
  wantedlists.clear();
  deletefiles.clear();
  wantedlists.push_back(target);
}

bool RecursiveCommandLogic::isActive() const {
  return active;
}

int RecursiveCommandLogic::getAction(const Path & currentpath, Path & actiontarget) {
  if (listtarget) {
    if (currentpath == listtargetpath) {
      listtarget = false;
      return RCL_ACTION_LIST;
    }
    else {
      actiontarget = listtargetpath;
      return RCL_ACTION_CWD;
    }
  }
  if (!wantedlists.size()) {
    if (!deletefiles.size()) {
      active = false;
      return RCL_ACTION_NOOP;
    }
    std::pair<Path, bool> delfile = deletefiles.back();
    if (currentpath != delfile.first.dirName()) {
      actiontarget = delfile.first.dirName();
      return RCL_ACTION_CWD;
    }
    actiontarget = delfile.first.baseName();
    deletefiles.pop_back();
    if (!deletefiles.size()) {
      active = false;
    }
    if (delfile.second) {
      return RCL_ACTION_DELDIR;
    }
    return RCL_ACTION_DELETE;
  }
  else {
    listtarget = true;
    listtargetpath = actiontarget = wantedlists.front();
    wantedlists.pop_front();
    if (currentpath == listtargetpath) {
      listtarget = false;
      return RCL_ACTION_LIST;
    }
    return RCL_ACTION_CWD;
  }
}

void RecursiveCommandLogic::addFileList(FileList * fl) {
  const Path & path = fl->getPath();
  if (!path.contains(target)) {
    return;
  }
  deletefiles.push_back(std::pair<Path, bool>(path, true));
  for (std::map<std::string, File *>::iterator it = fl->begin(); it != fl->end(); it++) {
    if (it->second->isDirectory()) {
      wantedlists.push_back(path / it->first);
    }
    else {
      deletefiles.push_back(std::pair<Path, bool>(path / it->first, false));
    }
  }
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}

void RecursiveCommandLogic::failedCwd() {
  listtarget = false;
  if (listtargetpath == basepath) {
    active = false;
  }
  else {
    deletefiles.push_back(std::pair<Path, bool>(listtargetpath, true));
  }
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}
