#pragma once

#include <string>
#include <list>
#include <map>

class ExternalFileViewing {
public:
  ExternalFileViewing();
  bool isViewable(std::string);
  int view(std::string);
  int viewThenDelete(std::string);
  void killProcess(int);
  void killAll();
  void hasDied(int);
  bool hasDisplay();
  bool stillViewing(int);
  std::string getVideoViewer();
  std::string getAudioViewer();
  std::string getImageViewer();
  std::string getPDFViewer();
  void setVideoViewer(std::string);
  void setAudioViewer(std::string);
  void setImageViewer(std::string);
  void setPDFViewer(std::string);
  void readConfiguration();
  void writeState();
  std::string getViewApplication(std::string);
private:
  std::string getExtension(std::string);
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

void sighandler_child(int);
