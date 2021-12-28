#include "externalscripts.h"

ExternalScript::ExternalScript(const std::string& name, const Path& path) :
  name(name), path(path)
{

}
std::list<ExternalScript>::const_iterator ExternalScripts::begin() const {
  return scripts.begin();
}

std::list<ExternalScript>::const_iterator ExternalScripts::end() const {
  return scripts.end();
}

void ExternalScripts::addScript(const std::string& name, const Path& path)
{
  scripts.emplace_back(name, path);
}

int ExternalScripts::size() const {
  return scripts.size();
}

void ExternalScripts::clear() {
  scripts.clear();
}
