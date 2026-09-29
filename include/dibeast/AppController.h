#pragma once

#include "dibeast/AppEnvironment.h"
#include "dibeast/Export.h"
#include "dibeast/Result.h"
#include "dibeast/Types.h"

#include <string_view>

namespace dibeast {

/**
 @brief DIAL application resource logic; all calls are serialized on the library's app strand.
 **/
class DIBEAST_EXPORT AppController {
 public:
  virtual ~AppController();

  virtual void attach(AppEnvironment& environment) = 0;

  virtual Result<Reply> info(const AppRequest& request) = 0;
  virtual Result<Reply> launch(const AppRequest& request) = 0;
  virtual Result<Reply> stop(const AppRequest& request) = 0;
  virtual Result<Reply> hide(const AppRequest& request) = 0;
  virtual Result<Reply> additionalData(const AppRequest& request) = 0;
  virtual void onExit(std::string_view appName, int exitCode) = 0;
};

}  // namespace dibeast
