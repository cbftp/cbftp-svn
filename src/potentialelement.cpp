#include "potentialelement.h"

PotentialElement::PotentialElement() {
  this->site = NULL;
  this->potential = 0;
  this->site_dnslots = 0;
}

SiteLogic * PotentialElement::getSite() const {
  return site;
}

int PotentialElement::getPotential() const {
  return potential;
}

void PotentialElement::update(SiteLogic * site, int site_dnslots, int potential, std::string filename) {
  this->site = site;
  this->site_dnslots = site_dnslots;
  this->potential = potential;
  this->filename = filename;
}

int PotentialElement::getSiteDownloadSlots() const {
  return site_dnslots;
}

std::string PotentialElement::getFileName() const {
  return filename;
}
