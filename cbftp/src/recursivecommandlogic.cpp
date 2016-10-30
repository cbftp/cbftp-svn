#include "recursivecommandlogic.h"

#include <algorithm>

#include "filelist.h"
#include "file.h"

bool lengthSort(const Path & a, const Path & b) {
  if (a.toString().length() < b.toString().length()) {
    return true;
  }
  return false;
}

RecursiveCommandLogic::RecursiveCommandLogic() {
  active = false;
}

void RecursiveCommandLogic::initialize(int mode, const Path & basepath, const std::string & target) {
  this->mode = mode;
  active = true;
  listtarget = false;
  this->basepath = basepath;
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
  }
  if (!wantedlists.size()) {
    if (!deletefiles.size()) {
      active = false;
      return RCL_ACTION_NOOP;
    }
    if (currentpath != basepath) {
      actiontarget = basepath;
      return RCL_ACTION_CWD;
    }
    actiontarget = deletefiles.back();
    deletefiles.pop_back();
    if (!deletefiles.size()) {
      active = false;
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
  deletefiles.push_back(path);
  for (std::map<std::string, File *>::iterator it = fl->begin(); it != fl->end(); it++) {
    if (it->second->isDirectory()) {
      wantedlists.push_back(path / it->first);
    }
    else {
      deletefiles.push_back(path / it->first);
    }
  }
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}

void RecursiveCommandLogic::failedCwd() {
  deletefiles.push_back(listtargetpath);
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}
