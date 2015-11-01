#pragma once

#include <string>

#include "pointer.h"
#include "eventreceiver.h"

class DataFileHandler;

class SettingsLoaderSaver : public EventReceiver {
public:
  SettingsLoaderSaver();
  bool enterKey(const std::string &);
  void saveSettings();
  bool dataExists() const;
  bool changeKey(const std::string &, const std::string &);
  void tick(int);
private:
  void loadSettings();
  void startAutoSaver();
  Pointer<DataFileHandler> dfh;
};
