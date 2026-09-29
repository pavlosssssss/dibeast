#pragma once

#include "dibeast/AppLauncher.h"
#include "dibeast/Result.h"
#include "dibeast/Types.h"

#include <boost/url/url.hpp>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace dibeast::app {

struct RegisteredApp {
  AppDescriptor descriptor;
  std::unique_ptr<AppLauncher> launcher;
  std::vector<boost::urls::url> allowedOrigins;
};

/**
 @brief Known applications by DIAL name; filled at startup, read-only afterwards.
 **/
class AppRegistry {
 public:
  VoidResult add(AppDescriptor descriptor, std::unique_ptr<AppLauncher> launcher);

  Result<const RegisteredApp*> find(std::string_view name) const;

 private:
  std::map<std::string, RegisteredApp, std::less<>> apps_;
};

}  // namespace dibeast::app
