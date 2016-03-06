#pragma once

#include <string>
#include <list>
#include <map>

#include "eventreceiver.h"

class ExternalFileViewing : public EventReceiver {
public:
  ExternalFileViewing();
  bool isViewable(std::string) const;
  int view(std::string);
  int viewThenDelete(std::string);
  void killProcess(int);
  void killAll();
  void signal(int, int);
  bool hasDisplay() const;
  bool stillViewing(int) const;
  std::string getVideoViewer() const;
  std::string getAudioViewer() const;
  std::string getImageViewer() const;
  std::string getPDFViewer() const;
  void setVideoViewer(std::string);
  void setAudioViewer(std::string);
  void setImageViewer(std::string);
  void setPDFViewer(std::string);
  std::string getViewApplication(std::string) const;
  static std::string getExtension(std::string);
private:
  int view(std::string, bool);
  void checkDeleteFile(int);
  std::list<int> subprocesses;
  std::map<int, std::string> files;
  std::string videoviewer;
  std::string audioviewer;
  std::string imageviewer;
  std::string pdfviewer;
  bool display;
};
