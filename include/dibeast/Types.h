#pragma once

#include "dibeast/Result.h"

#include <string>
#include <utility>
#include <vector>

namespace dibeast {

// Every member has a default initializer so designated init may skip any of them
// (keeps GCC -Wmissing-field-initializers quiet in client code).

/**
 @brief Identity of this DIAL device, used by SSDP and the device description.
 **/
struct DeviceInfo {
  std::string uuid{};
  std::string friendlyName{};
  std::string manufacturer{};
  std::string modelName{};
};

/**
 @brief Static description of a launchable DIAL application.
 **/
struct AppDescriptor {
  std::string name{};
  bool allowStop = true;
  std::vector<std::string> allowedOrigins{};
};

using AdditionalData = std::vector<std::pair<std::string, std::string>>;

enum class RunState { Stopped, Starting, Running, Hidden };

inline bool isRunning(RunState state) {
  return state != RunState::Stopped;
}

/**
 @brief Runtime state of one DIAL application.
 **/
struct AppState {
  RunState state = RunState::Stopped;
  std::string instance = "run";
  AdditionalData additionalData{};
};

/**
 @brief One DIAL REST call on an application resource, already decoded from HTTP.
 **/
struct AppRequest {
  std::string appName{};
  std::string instance{};
  std::string payload{};
  AdditionalData additionalData{};
  std::string clientDialVersion{};
  std::string applicationUrl{};
  std::string additionalDataUrl{};
  bool fromLocalhost = false;
};

/**
 @brief Transport-neutral successful controller reply.
 **/
struct Reply {
  enum class Status { Ok, Created };

  Status status = Status::Ok;
  std::string contentType{};
  std::string body{};
  Headers headers{};
};

}  // namespace dibeast
