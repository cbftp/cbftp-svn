#include "braillegraph.h"

#include <set>

#include "termint.h"

namespace {

const unsigned int BRAILLE_UNICODE_BASE = 0x2800;

}

BrailleGraph::BrailleGraph(unsigned int row, unsigned int col,
  const std::string& title, const std::string& unit, unsigned int floor,
  unsigned int ceiling) : row(row), col(col), min(0), max(0), avg(0),
  graphfloor(floor), graphceiling(ceiling), title(title), unit(unit)
{

}

void BrailleGraph::resize(unsigned int row, unsigned int col) {
  bool redraw = this->row != row || this->col != col;
  this->row = row;
  this->col = col;
  if (redraw) {
    render();
  }
}

void BrailleGraph::setData(const std::list<unsigned int>& data) {
  this->data = data;
  render();
}

unsigned int BrailleGraph::rows() const {
  return row;
}

unsigned int BrailleGraph::cols() const {
  return col;
}

unsigned int BrailleGraph::getChar(unsigned int row, unsigned int col) const {
  auto it = graph.find(row);
  if (it != graph.end()) {
    auto it2 = it->second.find(col);
    if (it2 != it->second.end()) {
      return it2->second;
    }
  }
  return 0;
}

void BrailleGraph::render() {
  graph.clear();
  unsigned int graphrows = 1;
  unsigned int graphcols = col;
  unsigned int graphrowstart = 0;
  unsigned int graphcolstart = 0;
  if (row > 3) {
    graphrows = row - 2;
    graphrowstart = 1;
    graphcols -= 2;
    graphcolstart = 1;
    for (unsigned int i = 0; i < graphcols; i++) {
      graph[0][i + 1] = BOX_HLINE;
      graph[row - 1][i + 1] = BOX_HLINE;
    }
    for (unsigned int i = 0; i < graphrows; i++) {
      graph[i + 1][0] = BOX_VLINE;
      graph[i + 1][col - 1] = BOX_VLINE;
    }
    graph[0][0] = BOX_CORNER_TL;
    graph[0][col - 1] = BOX_CORNER_TR;
    graph[row - 1][0] = BOX_CORNER_BL;
    graph[row - 1][col - 1] = BOX_CORNER_BR;
  }
  else if (row > 1) {
    graphrows = row - 1;
  }
  unsigned int biggraphcols = graphcols * 2;
  unsigned int biggraphrows = graphrows * 4;
  std::map<unsigned int, std::set<unsigned int>> biggraph;
  float colfactor = static_cast<float>(biggraphcols) / data.size();
  float rowfactor = static_cast<float>(biggraphrows) / (graphceiling - graphfloor + 1);
  int datapos = 0;
  min = graphceiling;
  max = graphfloor;
  avg = 0;
  unsigned int lastrow = 0;
  unsigned int lastcol = 0;
  bool first = true;
  for (unsigned int datapoint : data) {
    if (datapoint > graphceiling) {
      datapoint = graphceiling;
    }
    if (datapoint < graphfloor) {
      datapoint = graphfloor;
    }
    unsigned int datarow = biggraphrows - ((rowfactor * datapoint) + 0.5);
    unsigned int datacol = (colfactor * datapos) + 0.5;
    biggraph[datarow].insert(datacol);
    avg += datapoint;
    if (datapoint > max) {
      max = datapoint;
    }
    if (datapoint < min) {
      min = datapoint;
    }
    if (!first && (lastrow != datarow || lastcol != datacol - 1)) {
      unsigned int coldiff = datacol - lastcol;
      if (datarow > lastrow) {
        float incpercol = static_cast<float>(datarow - lastrow) / coldiff;
        float rowincreased = lastrow;
        for (unsigned int intercol = lastcol + 1; intercol <= datacol; intercol++) {
          for (unsigned int currinterrow = rowincreased; currinterrow < rowincreased + incpercol; currinterrow++) {
            biggraph[currinterrow].insert(intercol);
          }
          rowincreased += incpercol;
          if (rowincreased > datarow) {
            break;
          }
        }
      }
      else {
        float decpercol = static_cast<float>(lastrow - datarow) / coldiff;
        float rowdecreased = lastrow;
        for (unsigned int intercol = lastcol + 1; intercol <= datacol; intercol++) {
          for (int currinterrow = rowdecreased; currinterrow >= rowdecreased - decpercol && rowdecreased >= decpercol; currinterrow--) {
            biggraph[currinterrow].insert(intercol);
          }
          rowdecreased -= decpercol;
          if (rowdecreased < datarow) {
            break;
          }
        }
      }
    }
    ++datapos;
    lastrow = datarow;
    lastcol = datacol;
    first = false;
  }
  avg /= data.size();
  for (unsigned int j = 0; j < graphrows; j++) {
    for (unsigned int i = 0; i < graphcols; i++) {
      unsigned int bgrow = j * 4;
      unsigned int bgcol = i * 2;
      unsigned int val = BRAILLE_UNICODE_BASE;
      if (biggraph.count(bgrow) && biggraph.at(bgrow).count(bgcol)) {
        val += 0x1;
      }
      if (biggraph.count(bgrow + 1) && biggraph.at(bgrow + 1).count(bgcol)) {
        val += 0x2;
      }
      if (biggraph.count(bgrow + 2) && biggraph.at(bgrow + 2).count(bgcol)) {
        val += 0x4;
      }
      if (biggraph.count(bgrow) && biggraph.at(bgrow).count(bgcol + 1)) {
        val += 0x8;
      }
      if (biggraph.count(bgrow + 1) && biggraph.at(bgrow + 1).count(bgcol + 1)) {
        val += 0x10;
      }
      if (biggraph.count(bgrow + 2) && biggraph.at(bgrow + 2).count(bgcol + 1)) {
        val += 0x20;
      }
      if (biggraph.count(bgrow + 3) && biggraph.at(bgrow + 3).count(bgcol)) {
        val += 0x40;
      }
      if (biggraph.count(bgrow + 3) && biggraph.at(bgrow + 3).count(bgcol + 1)) {
        val += 0x80;
      }
      if (val != BRAILLE_UNICODE_BASE) {
        graph[j + graphrowstart][i + graphcolstart] = val;
      }
    }
  }


  std::string minstr = std::to_string(min);
  std::string avgstr = std::to_string(avg);
  std::string maxstr = std::to_string(max);
  std::string nowstr = std::to_string(data.front());
  if (row > 3) {
    std::string floorstr = std::to_string(graphfloor);
    std::string ceilingstr = std::to_string(graphceiling);
    unsigned int pos = 1;
    graph[0][pos++] = ' ';
    for (unsigned int i = 0; i < ceilingstr.length(); ++i) {
      graph[0][pos++] = ceilingstr[i];
    }
    graph[0][pos++] = ' ';
    pos = 1;
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < floorstr.length(); ++i) {
      graph[row - 1][pos++] = floorstr[i];
    }
    graph[row - 1][pos++] = ' ';
    pos = 10;
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < title.length(); ++i) {
      graph[row - 1][pos++] = title[i];
    }
    graph[row - 1][pos++] = ' ';
    ++pos;
    graph[row - 1][pos++] = ' ';
    graph[row - 1][pos++] = 'N';
    graph[row - 1][pos++] = 'o';
    graph[row - 1][pos++] = 'w';
    graph[row - 1][pos++] = ':';
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < nowstr.length(); ++i) {
      graph[row - 1][pos++] = nowstr[i];
    }
    for (unsigned int i = 0; i < unit.length(); ++i) {
      graph[row - 1][pos++] = unit[i];
    }
    graph[row - 1][pos++] = ' ';
    ++pos;
    graph[row - 1][pos++] = ' ';
    graph[row - 1][pos++] = 'M';
    graph[row - 1][pos++] = 'i';
    graph[row - 1][pos++] = 'n';
    graph[row - 1][pos++] = ':';
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < minstr.length(); ++i) {
      graph[row - 1][pos++] = minstr[i];
    }
    for (unsigned int i = 0; i < unit.length(); ++i) {
      graph[row - 1][pos++] = unit[i];
    }
    graph[row - 1][pos++] = ' ';
    pos++;
    graph[row - 1][pos++] = ' ';
    graph[row - 1][pos++] = 'A';
    graph[row - 1][pos++] = 'v';
    graph[row - 1][pos++] = 'g';
    graph[row - 1][pos++] = ':';
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < avgstr.length(); ++i) {
      graph[row - 1][pos++] = avgstr[i];
    }
    for (unsigned int i = 0; i < unit.length(); ++i) {
      graph[row - 1][pos++] = unit[i];
    }
    graph[row - 1][pos++] = ' ';
    pos++;
    graph[row - 1][pos++] = ' ';
    graph[row - 1][pos++] = 'M';
    graph[row - 1][pos++] = 'a';
    graph[row - 1][pos++] = 'x';
    graph[row - 1][pos++] = ':';
    graph[row - 1][pos++] = ' ';
    for (unsigned int i = 0; i < maxstr.length(); ++i) {
      graph[row - 1][pos++] = maxstr[i];
    }
    for (unsigned int i = 0; i < unit.length(); ++i) {
      graph[row - 1][pos++] = unit[i];
    }
    graph[row - 1][pos++] = ' ';
  }
}
