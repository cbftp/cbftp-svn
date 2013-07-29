#pragma once

class EngineBase {
private:
public:
  virtual ~EngineBase();
  virtual void someRaceFileListRefreshed() = 0;
};
