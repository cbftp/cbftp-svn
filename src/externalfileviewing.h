#pragma once

#include <string>
#include <list>
#include <map>

#include "core/eventreceiver.h"

class Path;

class ExternalFileViewing : public EventReceiver {
public:
  ExternalFileViewing();
  bool isViewable(const Path &) const;
  int view(const Path &);
  int viewThenDelete(const Path &);
  void killProcess(int);
  void killAll();
  void signal(int, int);
  bool hasDisplay() const;
  bool stillViewing(int) const;
  std::string getVideoViewer() const;
  std::string getAudioViewer() const;
  std::string getImageViewer() const;
  std::string getPDFViewer() const;
  void setVideoViewer(const std::string &);
  void setAudioViewer(const std::string &);
  void setImageViewer(const std::string &);
  void setPDFViewer(const std::string &);
  std::string getViewApplication(const Path &) const;
  static std::string getExtension(const std::string &);
private:
  int view(const Path &, bool);
  void checkDeleteFile(int);
  std::list<int> subprocesses;
  std::map<int, Path> files;
  std::string videoviewer;
  std::string audioviewer;
  std::string imageviewer;
  std::string pdfviewer;
  bool display;
};
