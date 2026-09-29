#include "DemoLauncher.h"
#include "ForkExecLauncher.h"
#include "LoggingAppController.h"
#include "phony/PhonyApp.h"

#include <dibeast/DialApp.h>

#include <memory>
#include <print>

int main() {
  dibeast::DialApp app{{
      .device =
          {
              .uuid = "2fac1234-31f8-11b4-a222-08002b34c003",
              .friendlyName = "DiBeast Server (DIAL on Boost.Beast)",
              .manufacturer = "pet-project",
              .modelName = "dial-asio",
          },
      .httpPort = 56789,
  }};

  // clang-format off
  for (auto configured : {app.addApp(dibeast::apps::makePhonyDescriptor(), app.makeProcessLauncher(DIBEAST_PHONY_EXECUTABLE)),
                          app.addApp({.name = "phony-fork"}, std::make_unique<ForkExecLauncher>(DIBEAST_PHONY_EXECUTABLE)),
                          app.addApp({.name = "demo"}, std::make_unique<DemoLauncher>()),
                          app.setAppController(std::make_unique<LoggingAppController>())}) {
    if (!configured) {
      std::println(stderr, "{}", configured.error().message);
      return 1;
    }
  }
  // clang-format on

  return app.exec();
}
