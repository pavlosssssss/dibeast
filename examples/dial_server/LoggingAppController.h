#pragma once

#include <dibeast/DefaultAppController.h>

#include <print>

/**
 @brief DefaultAppController that logs every launch request before handling it.
 **/
class LoggingAppController : public dibeast::DefaultAppController {
 public:
  dibeast::Result<dibeast::Reply> launch(const dibeast::AppRequest& request) override {
    std::println("controller: launch '{}' from {}, clientDialVer '{}'", request.appName,
                 request.fromLocalhost ? "localhost" : "network", request.clientDialVersion);

    return DefaultAppController::launch(request);
  }
};
