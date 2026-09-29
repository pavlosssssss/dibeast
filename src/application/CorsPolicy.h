#pragma once

#include "application/AppRegistry.h"
#include "dibeast/Result.h"

#include <string_view>

namespace dibeast::app {

/**
 @brief DIAL CORS policy (spec 6.6); stateless, safe to call from any thread.
 **/
class CorsPolicy {
 public:
  explicit CorsPolicy(const AppRegistry& registry);

  Result<bool> allowOrigin(std::string_view appName, std::string_view origin) const;

 private:
  const AppRegistry& registry_;
};

}  // namespace dibeast::app
