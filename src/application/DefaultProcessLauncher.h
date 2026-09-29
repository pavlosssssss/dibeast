#pragma once

#include "common/Strand.h"
#include "dibeast/AppLauncher.h"

#include <boost/process/process.hpp>

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace dibeast::app {

/**
 @brief AppLauncher that runs the app as a child process; used on the app strand only.
 **/
class DefaultProcessLauncher : public AppLauncher {
 public:
  DefaultProcessLauncher(IoStrand strand, std::string executable);

  VoidResult launch(const LaunchRequest& request, ExitCallback onExit) override;
  VoidResult stop(std::string_view appName) override;

 private:
  using Process = boost::process::basic_process<IoStrand>;

  IoStrand strand_;
  std::string executable_;
  std::map<std::string, std::unique_ptr<Process>, std::less<>> processes_;
};

}  // namespace dibeast::app
