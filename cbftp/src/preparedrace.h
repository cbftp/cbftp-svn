#pragma once

#include <string>
#include <list>

class PreparedRace {
public:
  PreparedRace(unsigned int, const std::string &, const std::string &, const std::list<std::string> &);
  unsigned int getId() const;
  const std::string & getRelease() const;
  const std::string & getSection() const;
  const std::list<std::string> & getSites() const;
  int getRemainingTime() const;
  void tick();
private:
  unsigned int id;
  const std::string release;
  const std::string section;
  const std::list<std::string> sites;
  int ttl;
};
