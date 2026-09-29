#pragma once

#include "dibeast/AppLauncher.h"
#include "dibeast/Export.h"
#include "dibeast/Types.h"

#include <string_view>

namespace dibeast {

/**
 @brief What the library offers an AppController: registered apps, their state and launchers.
 Controllers are only called for registered apps, so lookups by their name always succeed.
 **/
class DIBEAST_EXPORT AppEnvironment {
 public:
  virtual ~AppEnvironment();

  virtual const AppDescriptor& descriptor(std::string_view appName) const = 0;
  virtual AppState& state(std::string_view appName) = 0;
  virtual AppLauncher& launcher(std::string_view appName) = 0;
  virtual AppLauncher::ExitCallback exitCallback(std::string_view appName) = 0;
};

}  // namespace dibeast
