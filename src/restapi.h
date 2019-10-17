#pragma once

#include <list>
#include <set>

#include "core/eventreceiver.h"
#include "http/request.h"

#include "requestcallback.h"

class RestApiCallback;
class SiteLogic;

enum class OngoingRequestType {
  RAW_COMMAND,
  FILE_LIST
};

struct OngoingRequest {
  OngoingRequestType type;
  int connrequestid;
  int apirequestid;
  RestApiCallback* cb;
  int timepassed = 0;
  int timeout;
  bool async = false;
  std::set<std::pair<void*, int>> ongoingservicerequests;
  std::list<std::pair<void*, std::string>> successes;
  std::list<std::pair<void*, std::string>> failures;
};

class RestApi : private RequestCallback, private Core::EventReceiver {
public:
  RestApi();
  ~RestApi();
  void handleRequest(RestApiCallback* cb, int connrequestid, const http::Request& request);
private:
  void tick(int message) override;
  void requestReady(void* service, int servicerequestid) override;
  void respondAsynced(OngoingRequest& request);
  void finalize(OngoingRequest& request);
  OngoingRequest* findOngoingRequest(void* service, int servicerequestid);
  OngoingRequest* findOngoingRequest(int apirequestid);
  std::list<OngoingRequest> ongoingrequests;
  int nextrequestid;
};
