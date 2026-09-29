#pragma once

#include "dibeast/Export.h"
#include "dibeast/Result.h"

#include <functional>
#include <string>
#include <string_view>

namespace dibeast {

/**
 @brief Starts and stops a first-screen application; one instance per registered app.
 **/
class DIBEAST_EXPORT AppLauncher {
 public:
  using ExitCallback = std::function<void(int exitCode)>;

  struct LaunchRequest {
    std::string appName;
    std::string payload;
    std::string additionalDataUrl;
  };

  virtual ~AppLauncher();

  virtual VoidResult launch(const LaunchRequest& request, ExitCallback onExit) = 0;
  virtual VoidResult stop(std::string_view appName) = 0;
  virtual VoidResult hide(std::string_view appName);
};

}  // namespace dibeast
