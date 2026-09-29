#include "DefaultProcessLauncher.h"

#include "common/SystemError.h"

#include <boost/process/default_launcher.hpp>

#include <format>
#include <print>
#include <vector>

namespace dibeast::app {

DefaultProcessLauncher::DefaultProcessLauncher(IoStrand strand, std::string executable)
    : strand_{std::move(strand)}
    , executable_{std::move(executable)} {}

VoidResult DefaultProcessLauncher::launch(const LaunchRequest& request, ExitCallback onExit) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  const auto& name = request.appName;

  if (processes_.contains(name))
    return Err(ErrorCode::Internal, "process '{}' is already running", name);

  const std::vector<std::string> args{
      "--dial-payload",
      request.payload,
      "--additional-data-url",
      request.additionalDataUrl,
  };

  boost::system::error_code ec;
  auto process = std::make_unique<Process>(
      boost::process::default_process_launcher()(strand_, ec, executable_, args));

  if (auto spawned =
          toResult(ec, std::format("spawn '{}'", executable_), ErrorCode::ServiceUnavailable);
      !spawned)
    return spawned;

  std::println("process: '{}' started, pid {}", name, process->id());

  process->async_wait(
      [this, name, onExit = std::move(onExit)](boost::system::error_code ec, int exitCode) {
        DIBEAST_ASSERT_ON_STRAND(strand_);
        std::println("process: '{}' exited, code {} {}", name, exitCode, ec ? ec.message() : "");
        processes_.erase(name);
        onExit(exitCode);
      });

  processes_.emplace(name, std::move(process));

  return Ok();
}

VoidResult DefaultProcessLauncher::stop(std::string_view appName) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  auto it = processes_.find(appName);

  if (it == processes_.end())
    return Err(ErrorCode::NotFound, "process '{}' is not running", appName);

  boost::system::error_code ec;
  it->second->request_exit(ec);

  return toResult(ec, std::format("stop '{}'", appName));
}

}  // namespace dibeast::app
