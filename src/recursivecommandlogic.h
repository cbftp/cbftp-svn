#pragma once

#define RCL_DELETE 0;
#define RCL_DELETEOWN 1;
#define RCL_TRANSFER 2;

class RecursiveCommandLogic {
private:
  int mode;
  bool complete;
public:
  RecursiveCommandLogic();
  void initialize(int);
  bool completed();
  std::string getRelevantPath();
};
