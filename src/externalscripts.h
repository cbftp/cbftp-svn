#pragma once

#include <list>
#include <string>

#include "path.h"

struct ExternalScript {
  ExternalScript(const std::string& name, const Path& path);
  std::string name;
  Path path;
};

class ExternalScripts {
public:
  std::list<ExternalScript>::const_iterator begin() const;
  std::list<ExternalScript>::const_iterator end() const;
  void addScript(const std::string& name, const Path& path);
  int size() const;
  void clear();
private:
  std::list<ExternalScript> scripts;
};
