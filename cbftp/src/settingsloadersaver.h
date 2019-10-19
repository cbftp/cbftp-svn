#pragma once

#include <list>
#include <memory>
#include <string>

#include "core/eventreceiver.h"

#include "datafilehandler.h"

class SkipList;

class SettingsAdder {
public:
  virtual ~SettingsAdder() {
  }
  virtual void loadSettings(std::shared_ptr<DataFileHandler>) = 0;
  virtual void saveSettings(std::shared_ptr<DataFileHandler>) = 0;
};

class SettingsLoaderSaver : public Core::EventReceiver {
public:
  SettingsLoaderSaver();
  DataFileState getState() const;
  bool tryDecrypt(const std::string& key);
  void init();
  void saveSettings();
  bool changeKey(const std::string& key, const std::string& newkey);
  bool setEncrypted(const std::string& key);
  bool setPlain(const std::string& key);
  void tick(int);
  void addSettingsAdder(SettingsAdder *);
  void removeSettingsAdder(SettingsAdder *);
private:
  void loadSettings();
  void startAutoSaver();
  void addSkipList(const SkipList *, const std::string &, const std::string &);
  void loadSkipListEntry(SkipList * skiplist, std::string value);
  std::shared_ptr<DataFileHandler> dfh;
  std::list<SettingsAdder *> settingsadders;
  bool loaded;
};
