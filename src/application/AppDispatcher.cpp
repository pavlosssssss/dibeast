#include "AppDispatcher.h"

#include "dibeast/DefaultAppController.h"

#include <boost/asio/post.hpp>

namespace dibeast::app {

namespace asio = boost::asio;

AppDispatcher::AppDispatcher(IoStrand strand, const AppRegistry& registry,
                             data::AppStateStore& store)
    : strand_{std::move(strand)}
    , registry_{registry}
    , store_{store}
    , controller_{std::make_unique<DefaultAppController>()}
    , exitGate_{std::make_shared<ExitGate>(this)} {}

AppDispatcher::~AppDispatcher() {
  std::lock_guard lock{exitGate_->mutex};
  exitGate_->dispatcher = nullptr;
}

void AppDispatcher::setController(std::unique_ptr<AppController> controller) {
  controller_ = std::move(controller);
}

void AppDispatcher::attach() {
  controller_->attach(*this);
}

void AppDispatcher::dispatch(Operation operation, AppRequest request, Completion done) {
  if (auto app = registry_.find(request.appName); !app)
    return done(Err(app.error()));

  asio::post(strand_,
             [this, operation, request = std::move(request), done = std::move(done)]() mutable {
               DIBEAST_ASSERT_ON_STRAND(strand_);
               done((controller_.get()->*operation)(request));
             });
}

const AppDescriptor& AppDispatcher::descriptor(std::string_view appName) const {
  auto app = registry_.find(appName);
  DIBEAST_ASSERT(app.has_value());

  return (*app)->descriptor;
}

AppState& AppDispatcher::state(std::string_view appName) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  return store_.state(appName);
}

AppLauncher& AppDispatcher::launcher(std::string_view appName) {
  DIBEAST_ASSERT_ON_STRAND(strand_);
  auto app = registry_.find(appName);
  DIBEAST_ASSERT(app.has_value());

  return *(*app)->launcher;
}

AppLauncher::ExitCallback AppDispatcher::exitCallback(std::string_view appName) {
  return [gate = exitGate_, name = std::string{appName}](int exitCode) {
    std::lock_guard lock{gate->mutex};

    if (gate->dispatcher != nullptr)
      gate->dispatcher->postExit(name, exitCode);
  };
}

void AppDispatcher::postExit(std::string name, int exitCode) {
  // Launchers may report the exit from any thread; the controller only runs on the strand.
  asio::post(strand_, [this, name = std::move(name), exitCode] {
    DIBEAST_ASSERT_ON_STRAND(strand_);
    controller_->onExit(name, exitCode);
  });
}

}  // namespace dibeast::app
