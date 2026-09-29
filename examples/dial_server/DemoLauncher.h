#pragma once

#include <dibeast/AppLauncher.h>

#include <print>
#include <utility>

/**
 @brief In-memory AppLauncher: nothing is started, it only logs and supports hide.
 **/
class DemoLauncher : public dibeast::AppLauncher {
 public:
  dibeast::VoidResult launch(const LaunchRequest& request, ExitCallback onExit) override {
    std::println("demo: launch '{}' payload='{}'", request.appName, request.payload);
    onExit_ = std::move(onExit);

    return dibeast::Ok();
  }

  dibeast::VoidResult stop(std::string_view appName) override {
    std::println("demo: stop '{}'", appName);

    if (auto onExit = std::exchange(onExit_, nullptr))
      onExit(0);

    return dibeast::Ok();
  }

  dibeast::VoidResult hide(std::string_view appName) override {
    std::println("demo: hide '{}'", appName);

    return dibeast::Ok();
  }

 private:
  ExitCallback onExit_;
};
