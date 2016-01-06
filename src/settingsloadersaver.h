#pragma once

#include <string>
#include <list>

#include "pointer.h"
#include "eventreceiver.h"

class DataFileHandler;

class SettingsAdder {
public:
  virtual ~SettingsAdder() {
  }
  virtual void loadSettings(Pointer<DataFileHandler>) = 0;
  virtual void saveSettings(Pointer<DataFileHandler>) = 0;
};

class SettingsLoaderSaver : public EventReceiver {
public:
  SettingsLoaderSaver();
  bool enterKey(const std::string &);
  void saveSettings();
  bool dataExists() const;
  bool changeKey(const std::string &, const std::string &);
  void tick(int);
  void addSettingsAdder(SettingsAdder *);
  void removeSettingsAdder(SettingsAdder *);
private:
  void loadSettings();
  void startAutoSaver();
  Pointer<DataFileHandler> dfh;
  std::list<SettingsAdder *> settingsadders;
};
