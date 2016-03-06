#include "externalfileviewing.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <cctype>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "globalcontext.h"
#include "util.h"
#include "workmanager.h"

#include "localstorage.h"
#include "eventlog.h"

extern GlobalContext * global;

namespace {

static ExternalFileViewing * instance = NULL;

void sighandler(int signal) {
  global->getWorkManager()->dispatchSignal(instance, signal, 0);
}

}

ExternalFileViewing::ExternalFileViewing() {
  util::assert(instance == NULL);
  instance = this;
  videoviewer = "mplayer";
  audioviewer = "mplayer";
  imageviewer = "eog";
  pdfviewer = "evince";
  display = false;
  char * displayenv = getenv("DISPLAY");
  if (displayenv != NULL) {
    display = true;
  }
  struct sigaction sa;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGCHLD);
  sa.sa_handler = sighandler;
  sigaction(SIGCHLD, &sa, NULL);
}

bool ExternalFileViewing::isViewable(std::string path) const {
  return getViewApplication(path) != "";
}

int ExternalFileViewing::view(std::string path) {
  return view(path, false);
}

int ExternalFileViewing::viewThenDelete(std::string path) {
  return view(path, true);
}

int ExternalFileViewing::view(std::string path, bool deleteafter) {
  std::string application = getViewApplication(path);
  global->getEventLog()->log("ExternalFileViewing", "Opening " + path + " with " + application);
  int pid = fork();
  if (!pid) {
    int devnull = open("/dev/null", O_WRONLY);
    dup2(devnull, STDOUT_FILENO);
    dup2(devnull, STDERR_FILENO);
    execlp(application.c_str(), application.c_str(), path.c_str(), (char *)0);
  }
  else {
    if (deleteafter) {
      files[pid] = path;
    }
    subprocesses.push_back(pid);
  }
  return pid;
}

void ExternalFileViewing::killProcess(int pid) {
  for (std::list<int>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    if (*it == pid) {
      kill(pid, SIGHUP);
      checkDeleteFile(*it);
      subprocesses.erase(it);
      break;
    }
  }
}

void ExternalFileViewing::killAll() {
  for (std::list<int>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    kill(*it, SIGHUP);
    checkDeleteFile(*it);
  }
  subprocesses.clear();
}

std::string ExternalFileViewing::getViewApplication(std::string path) const {
  std::string extension = getExtension(path);
  std::string application;
  if (extension == "mkv" || extension == "mp4" || extension == "avi" ||
      extension == "wmv" || extension == "vob" || extension == "mov" ||
      extension == "mpg" || extension == "mpeg") {
    application = getVideoViewer();
  }
  else if (extension == "mp3" || extension == "wav" || extension == "flac" ||
      extension == "ogg" || extension == "wma" || extension == "mid") {
    application = getAudioViewer();
  }
  else if (extension == "png" || extension == "gif" || extension == "jpeg" ||
      extension == "jpg" || extension == "bmp") {
    application = getImageViewer();
  }
  else if (extension == "pdf") {
    application = getPDFViewer();
  }
  return application;
}

std::string ExternalFileViewing::getExtension(std::string file) {
  size_t suffixdotpos = file.rfind(".");
  std::string extension;
  if (suffixdotpos != std::string::npos && suffixdotpos > 0) {
    extension = file.substr(suffixdotpos + 1);
  }
  for (unsigned int i = 0; i < extension.length(); i++) {
    extension[i] = tolower(extension[i]);
  }
  return extension;
}

void ExternalFileViewing::signal(int signal, int) {
  if (signal == SIGCHLD) {
    int pid = 0;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
      for (std::list<int>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
        if (*it == pid) {
          checkDeleteFile(*it);
          subprocesses.erase(it);
          break;
        }
      }
    }
  }
  else {
    util::assert(false);
  }
}

void ExternalFileViewing::checkDeleteFile(int pid) {
  if (files.find(pid) != files.end()) {
    global->getEventLog()->log("ExternalFileViewing", "Deleting temporary file: " + files[pid]);
    global->getLocalStorage()->deleteFile(files[pid]);
    files.erase(pid);
  }
}

bool ExternalFileViewing::hasDisplay() const {
  return display;
}

bool ExternalFileViewing::stillViewing(int pid) const {
  for (std::list<int>::const_iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    if (*it == pid) {
      return true;
    }
  }
  return false;
}

std::string ExternalFileViewing::getVideoViewer() const {
  return videoviewer;
}

std::string ExternalFileViewing::getAudioViewer() const {
  return audioviewer;
}

std::string ExternalFileViewing::getImageViewer() const {
  return imageviewer;
}

std::string ExternalFileViewing::getPDFViewer() const {
  return pdfviewer;
}

void ExternalFileViewing::setVideoViewer(std::string viewer) {
  videoviewer = viewer;
}

void ExternalFileViewing::setAudioViewer(std::string viewer) {
  audioviewer = viewer;
}

void ExternalFileViewing::setImageViewer(std::string viewer) {
  imageviewer = viewer;
}

void ExternalFileViewing::setPDFViewer(std::string viewer) {
  pdfviewer = viewer;
}
