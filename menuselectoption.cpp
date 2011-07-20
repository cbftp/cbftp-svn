#include "menuselectoption.h"

MenuSelectOption::MenuSelectOption(WINDOW * window) {
  this->window = window;
  pointer = 0;
  maxheight = 0;
}

void MenuSelectOption::goNext() {
  print(pointer-1, false);
  if (pointer < options.size()) pointer++;
  else if (options.size() > 0) pointer = 1;
  print(pointer-1, true);
  wrefresh(window);
}

void MenuSelectOption::goPrev() {
  print(pointer-1, false);
  if (pointer > 0) pointer--;
  if (pointer == 0) pointer = options.size();
  print(pointer-1, true);
  wrefresh(window);
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext) {
  if (options.size() == 0 && pointer == 0) pointer++;
  options.push_back(MenuSelectOptionElement(row, col, identifier, label, starttext));
}

void MenuSelectOption::addIntArrowField(int row, int col, std::string identifier, std::string label, int startval) {
  if (options.size() == 0 && pointer == 0) pointer++;
  options.push_back(MenuSelectOptionElement(row, col, identifier, label, startval));
}

int MenuSelectOption::getSelectionDataCol() {
  return options[pointer-1].getCol() + options[pointer-1].getLabel().length() + 1;
}

int MenuSelectOption::getSelectionDataRow() {
  return options[pointer-1].getRow();
}

MenuSelectOptionElement & MenuSelectOption::getSelection() {
  return options[pointer-1];
}

void MenuSelectOption::clear() {
  options.clear();
}

void MenuSelectOption::print() {
  for (int i = 0; i < options.size(); i++) {
    while (pointer > options.size()) pointer--;
    print(i, i+1 == pointer);
  }
}

void MenuSelectOption::print(int index, bool highlight) {
  if (index < 0) return;
  std::string labelout = options[index].getLabel();
  std::string dataout = " " + (options[index].hasStrValue() ? options[index].getContent() : global->int2Str(options[index].getIntContent()));
  if (highlight) wattron(window, A_REVERSE);
  mvwprintw(window, options[index].getRow(), options[index].getCol(), labelout.c_str());
  if (highlight) wattroff(window, A_REVERSE);
  mvwprintw(window, options[index].getRow(), options[index].getCol() + labelout.length(), dataout.c_str());
}
