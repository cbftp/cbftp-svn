#include "preparedrace.h"

#define PREPARED_RACE_EXPIRATION_TIME 120

PreparedRace::PreparedRace(unsigned int id, const std::string & release, const std::string & section, const std::list<std::string> & sites) :
  id(id),
  release(release),
  section(section),
  sites(sites),
  ttl(PREPARED_RACE_EXPIRATION_TIME) {

}

unsigned int PreparedRace::getId() const {
  return id;
}

const std::string & PreparedRace::getRelease() const {
  return release;
}

const std::string & PreparedRace::getSection() const {
  return section;
}

const std::list<std::string> & PreparedRace::getSites() const {
  return sites;
}

int PreparedRace::getRemainingTime() const {
  return ttl;
}

void PreparedRace::tick() {
  if (ttl >= 0) {
    ttl -= 1;
  }
}
