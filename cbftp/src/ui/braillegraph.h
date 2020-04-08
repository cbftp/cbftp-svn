#pragma once

#include <list>
#include <map>
#include <string>

class BrailleGraph {
public:
  BrailleGraph(unsigned int row, unsigned int col, const std::string& title, const std::string& unit, unsigned int floor, unsigned int ceiling);
  void resize(unsigned int row, unsigned int col);
  void setData(const std::list<unsigned int>& data);
  unsigned int rows() const;
  unsigned int cols() const;
  unsigned int getChar(unsigned int row, unsigned int col) const;
private:
  void render();
  unsigned int row;
  unsigned int col;
  std::list<unsigned int> data;
  unsigned int min;
  unsigned int max;
  unsigned int avg;
  unsigned int graphfloor;
  unsigned int graphceiling;
  std::string title;
  std::string unit;
  std::map<unsigned int, std::map<unsigned int, unsigned int>> graph;
};
