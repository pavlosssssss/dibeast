#pragma once

#include "application/AppRegistry.h"
#include "common/Strand.h"
#include "data/AppStateStore.h"
#include "dibeast/AppController.h"
#include "dibeast/AppEnvironment.h"

#include <functional>
#include <memory>
#include <mutex>

namespace dibeast::app {

/**
 @brief Runs the AppController on the app strand and serves it as its AppEnvironment.
 **/
class AppDispatcher : public AppEnvironment {
 public:
  using Completion = std::move_only_function<void(Result<Reply>)>;
  using Operation = Result<Reply> (AppController::*)(const AppRequest&);

  AppDispatcher(IoStrand strand, const AppRegistry& registry, data::AppStateStore& store);
  ~AppDispatcher() override;

  void setController(std::unique_ptr<AppController> controller);
  void attach();

  void dispatch(Operation operation, AppRequest request, Completion done);

  const AppDescriptor& descriptor(std::string_view appName) const override;
  AppState& state(std::string_view appName) override;
  AppLauncher& launcher(std::string_view appName) override;
  AppLauncher::ExitCallback exitCallback(std::string_view appName) override;

 private:
  // Exit callbacks may outlive the dispatcher (e.g. called by a launcher thread during shutdown).
  struct ExitGate {
    AppDispatcher* dispatcher;
    std::mutex mutex;
  };

  void postExit(std::string name, int exitCode);

  IoStrand strand_;
  const AppRegistry& registry_;
  data::AppStateStore& store_;
  std::unique_ptr<AppController> controller_;
  std::shared_ptr<ExitGate> exitGate_;
};

}  // namespace dibeast::app
