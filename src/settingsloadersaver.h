#pragma once

#include <list>
#include <memory>
#include <string>

#include "core/eventreceiver.h"

class DataFileHandler;
class SkipList;
class SettingsAdder {
public:
  virtual ~SettingsAdder() {
  }
  virtual void loadSettings(std::shared_ptr<DataFileHandler>) = 0;
  virtual void saveSettings(std::shared_ptr<DataFileHandler>) = 0;
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
  void addSkipList(const SkipList *, const std::string &, const std::string &);
  void loadSkipListEntry(SkipList *, std::string);
  std::shared_ptr<DataFileHandler> dfh;
  std::list<SettingsAdder *> settingsadders;
};
