#pragma once

#include <list>
#include <string>
#include <memory>

#include "../../core/eventreceiver.h"

#include "../../path.h"

class BrowseScreenAction;
class ResizableElement;
class MenuSelectOption;
class UIFileList;
class UIFile;
class Ui;

enum class BrowseScreenType {
  SITE,
  LOCAL,
  SELECTOR
};

enum class LastInfo {
  CWD_FAILED,
  CHANGED_SORT,
  DELETE_SUCCESS,
  DELETE_FAILED,
  WIPE_SUCCESS,
  WIPE_FAILED,
  NUKE_SUCCESS,
  NUKE_FAILED,
  MKDIR_SUCCESS,
  MKDIR_FAILED,
  NONE
};

enum class BrowseScreenRequestType {
  FILELIST,
  PATH_INFO,
  DELETE,
  WIPE,
  NUKE,
  MKDIR
};

struct BrowseScreenRequest {
  int id;
  BrowseScreenRequestType type;
  Path path;
  std::list<std::pair<std::string, bool>> files;
};

class BrowseScreenSub : public Core::EventReceiver {
public:
  virtual void redraw(unsigned int, unsigned int, unsigned int) = 0;
  virtual void update() = 0;
  virtual BrowseScreenType type() const = 0;
  virtual std::string getLegendText() const = 0;
  virtual std::string getInfoLabel() const = 0;
  virtual std::string getInfoText() const = 0;
  virtual void command(const std::string & command, const std::string & arg);
  virtual BrowseScreenAction keyPressed(unsigned int) = 0;
  virtual void setFocus(bool) = 0;
  virtual UIFileList * getUIFileList();
  static void addFileDetails(MenuSelectOption & table, unsigned int coloffset,
                             unsigned int y, const std::string & name,
                             const std::string & prepchar = "", const std::string & size = "",
                             const std::string & lastmodified = "", const std::string & owner = "",
                             bool selectable = false, bool cursored = false, UIFile * origin = nullptr);
  static void printFlipped(Ui * ui, const std::shared_ptr<ResizableElement> & re);
  static std::string targetName(const std::list<std::pair<std::string, bool>> & items);
};
