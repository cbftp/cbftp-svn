#include "uifile.h"

UIFile::UIFile(File * file) {
  name = file->getName();
  directory = file->isDirectory();
  owner = file->getOwner();
  group = file->getGroup();
  size = file->getSize();
  lastmodified = file->getLastModified();
  selected = false;
  cursored = false;
}

std::string UIFile::getName() {
  return name;
}

std::string UIFile::getOwner() {
  return owner;
}

std::string UIFile::getGroup() {
  return group;
}

std::string UIFile::getLastModified() {
  return lastmodified;
}

long int UIFile::getSize() {
  return size;
}

bool UIFile::isDirectory() {
  return directory;
}

bool UIFile::isSelected() {
  return selected;
}

bool UIFile::isCursored() {
  return cursored;
}

void UIFile::select() {
  selected = true;
}

void UIFile::unSelect() {
  selected = false;
}

void UIFile::cursor() {
  cursored = true;
}

void UIFile::unCursor() {
  cursored = false;
}
