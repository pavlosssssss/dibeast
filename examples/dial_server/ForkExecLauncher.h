#pragma once

#include <dibeast/AppLauncher.h>

#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <thread>

/**
 @brief AppLauncher on plain fork/execv; one waiter thread per child reports its exit.
 **/
class ForkExecLauncher : public dibeast::AppLauncher {
 public:
  explicit ForkExecLauncher(std::string executable);
  ~ForkExecLauncher() override;

  dibeast::VoidResult launch(const LaunchRequest& request, ExitCallback onExit) override;
  dibeast::VoidResult stop(std::string_view appName) override;

 private:
  struct Child {
    pid_t pid = -1;
    std::jthread waiter;
  };

  dibeast::Result<pid_t> spawn(const LaunchRequest& request);
  void waitForExit(std::string name, pid_t pid, ExitCallback onExit);

  std::string executable_;
  std::mutex mutex_;
  std::map<std::string, Child, std::less<>> children_;
};
