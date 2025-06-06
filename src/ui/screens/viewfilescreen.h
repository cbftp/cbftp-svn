#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../uiwindow.h"

#include "../../core/types.h"

#include "../../encoding.h"
#include "../../path.h"

class TransferStatus;
class SiteLogic;
class FileList;
class Ui;

#define MAXOPENSIZE 524288

class ViewFileScreen : public UIWindow {
public:
  ViewFileScreen(Ui * ui);
  ~ViewFileScreen();
  void initialize(unsigned int, unsigned int, const std::string &, const std::string &, const std::shared_ptr<FileList>& fl);
  void initialize(unsigned int, unsigned int, const Path &, const std::string &);
  void redraw() override;
  void update() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  int getCurrentScope() const;
  int state;
  int requestid;
  std::shared_ptr<SiteLogic> sitelogic;
  std::shared_ptr<TransferStatus> ts;
  std::shared_ptr<FileList> filelist;
  std::string site;
  std::string file;
  unsigned int x;
  unsigned int xmax;
  unsigned int y;
  unsigned int ymax;
  unsigned long long int size;
  bool externallyviewable;
  bool legendupdated;
  bool deleteafter;
  std::vector<std::string > rawcontents;
  std::vector<std::basic_string<char32_t> > translatedcontents;
  Path path;
  int pid;
  encoding::Encoding encoding;
  Core::BinaryData tmpdata;
  bool goDown();
  bool goUp();
  void printTransferInfo();
  void translate();
  void initialize();
  void loadViewer();
  void viewInternal();
  void viewExternal();
};
