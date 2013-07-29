#include "potentialelement.h"

PotentialElement::PotentialElement() {
  this->site = NULL;
  this->potential = 0;
  this->site_dnslots = 0;
}

SiteLogic * PotentialElement::getSite() {
  return site;
}

int PotentialElement::getPotential() {
  return potential;
}

void PotentialElement::update(SiteLogic * site, int site_dnslots, int potential, std::string filename) {
  this->site = site;
  this->site_dnslots = site_dnslots;
  this->potential = potential;
  this->filename = filename;
}

int PotentialElement::getSiteDownloadSlots() {
  return site_dnslots;
}

std::string PotentialElement::getFileName() {
  return filename;
}
