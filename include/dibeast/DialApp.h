#pragma once

#include "dibeast/AppController.h"
#include "dibeast/AppLauncher.h"
#include "dibeast/Export.h"
#include "dibeast/Result.h"
#include "dibeast/Types.h"

#include <cstdint>
#include <memory>
#include <string>

namespace dibeast {

/**
 @brief DIAL server: SSDP discovery + REST service for the registered apps.
 **/
class DIBEAST_EXPORT DialApp {
 public:
  struct Config {
    DeviceInfo device{};
    std::uint16_t httpPort = 56789;
    unsigned threads = 0;
  };

  explicit DialApp(Config config);
  ~DialApp();

  DialApp(const DialApp&) = delete;
  DialApp& operator=(const DialApp&) = delete;

  std::unique_ptr<AppLauncher> makeProcessLauncher(std::string executable);
  VoidResult addApp(AppDescriptor descriptor, std::unique_ptr<AppLauncher> launcher);
  VoidResult setAppController(std::unique_ptr<AppController> controller);

  int exec();
  void stop();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace dibeast
