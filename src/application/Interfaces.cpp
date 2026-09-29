#include "dibeast/AppController.h"
#include "dibeast/AppEnvironment.h"
#include "dibeast/AppLauncher.h"

namespace dibeast {

AppLauncher::~AppLauncher() = default;

VoidResult AppLauncher::hide(std::string_view appName) {
  return Err(ErrorCode::NotImplemented, "hide of '{}' is not implemented", appName);
}

AppEnvironment::~AppEnvironment() = default;

AppController::~AppController() = default;

}  // namespace dibeast
