#include <string>

class Logger {
public:
  virtual ~Logger() { }
  virtual void log(const std::string &, const std::string &) = 0;
};
