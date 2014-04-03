#include "chardraw.h"

void CharDraw::init() {
  std::string init = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (unsigned int i = 0; i < init.length(); i++) {
    charmap[init[i]] = std::vector<std::string>();
  }
  charmap['A'].push_back(" AHHB ");
  charmap['A'].push_back("AdxxcB");
  charmap['A'].push_back("Vxabxv");
  charmap['A'].push_back("Vxcdxv");
  charmap['A'].push_back("Vxabxv");
  charmap['A'].push_back("ChDChD");
  charmap['H'].push_back("AHBAHB");
  charmap['H'].push_back("VxvVxv");
  charmap['H'].push_back("Vxcdxv");
  charmap['H'].push_back("Vxabxv");
  charmap['H'].push_back("VxvVxv");
  charmap['H'].push_back("ChDChD");
  charmap['P'].push_back("AHHHHB");
  charmap['P'].push_back("Vxabxv");
  charmap['P'].push_back("Vxcdxv");
  charmap['P'].push_back("VxahhD");
  charmap['P'].push_back("Vxv   ");
  charmap['P'].push_back("ChD   ");
  charmap['T'].push_back("AHHHHB");
  charmap['T'].push_back("ChbahD");
  charmap['T'].push_back("  Vv  ");
  charmap['T'].push_back("  Vv  ");
  charmap['T'].push_back("  Vv  ");
  charmap['T'].push_back("  CD  ");
}

std::string CharDraw::getCharLine(char c, int line) {
  return charmap[c][line];
}

std::map<char, std::vector< std::string > > CharDraw::charmap = std::map<char, std::vector< std::string > >();
