#include "section.h"

#include "globalcontext.h"

Section::Section() : jobs(0), skiplist(global->getSkipList()) {

}

Section::Section(const std::string & name) : name(name), jobs(0), skiplist(global->getSkipList()) {

}

std::string Section::getName() const {
  return name;
}

SkipList & Section::getSkipList() {
  return skiplist;
}

const SkipList & Section::getSkipList() const {
  return skiplist;
}

int Section::getNumJobs() const {
  return jobs;
}

void Section::setName(const std::string & newname) {
  name = newname;
}

void Section::setNumJobs(int jobs) {
  this->jobs = jobs;
}

void Section::addJob() {
  ++jobs;
}
