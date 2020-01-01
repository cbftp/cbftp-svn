#pragma once

#include <string>
#include <list>
#include <utility>

class PreparedRace {
public:
  PreparedRace(unsigned int, const std::string& release, const std::string& section, const std::list<std::pair<std::string, bool>>& sites, int);
  unsigned int getId() const;
  const std::string& getRelease() const;
  const std::string& getSection() const;
  const std::list<std::pair<std::string, bool>>& getSites() const;
  int getRemainingTime() const;
  void tick();
private:
  unsigned int id;
  const std::string release;
  const std::string section;
  const std::list<std::pair<std::string, bool>> sites;
  int ttl;
};
