#include "dibeast/DefaultAppController.h"

#include "application/DialXml.h"
#include "common/Assert.h"
#include "common/Constants.h"

#include <charconv>
#include <format>
#include <print>

namespace dibeast {

namespace {

bool reportsHidden(std::string_view clientDialVersion) {
  int major = 0;
  int minor = 0;
  const auto dot = clientDialVersion.find('.');
  const auto majorEnd = dot == std::string_view::npos ? clientDialVersion.size() : dot;
  std::from_chars(clientDialVersion.data(), clientDialVersion.data() + majorEnd, major);

  if (dot != std::string_view::npos)
    std::from_chars(clientDialVersion.data() + dot + 1,
                    clientDialVersion.data() + clientDialVersion.size(), minor);

  return major > 2 || (major == 2 && minor >= 1);
}

Result<const AppState*> findRunning(AppEnvironment& env, const AppRequest& request) {
  const auto& state = env.state(request.appName);

  if (!isRunning(state.state) || request.instance != state.instance)
    return Err(ErrorCode::NotFound, "no running instance '{}' of '{}'", request.instance,
               request.appName);

  return Ok(&state);
}

}  // namespace

struct DefaultAppController::Impl {
  AppEnvironment* environment = nullptr;
};

DefaultAppController::DefaultAppController()
    : impl_{std::make_unique<Impl>()} {}

DefaultAppController::~DefaultAppController() = default;

void DefaultAppController::attach(AppEnvironment& environment) {
  impl_->environment = &environment;
}

AppEnvironment& DefaultAppController::environment() {
  DIBEAST_ASSERT(impl_->environment != nullptr);

  return *impl_->environment;
}

Result<Reply> DefaultAppController::info(const AppRequest& request) {
  auto& env = environment();
  const auto& state = env.state(request.appName);
  const auto shownState =
      state.state == RunState::Hidden && !reportsHidden(request.clientDialVersion)
          ? RunState::Stopped
          : state.state;

  return Ok(Reply{
      .status = Reply::Status::Ok,
      .contentType = std::string{kXmlContentType},
      .body = app::renderServiceXml(env.descriptor(request.appName), state, shownState),
  });
}

Result<Reply> DefaultAppController::launch(const AppRequest& request) {
  auto& env = environment();
  const auto& name = request.appName;
  auto& state = env.state(name);

  if (!isRunning(state.state)) {
    const AppLauncher::LaunchRequest launch{
        .appName = name,
        .payload = request.payload,
        .additionalDataUrl = request.additionalDataUrl,
    };

    if (auto launched = env.launcher(name).launch(launch, env.exitCallback(name)); !launched)
      return Err(launched.error());
  }

  state.state = RunState::Running;

  return Ok(Reply{
      .status = Reply::Status::Created,
      .headers = {{"Location",
                   std::format("{}/{}/{}", request.applicationUrl, name, state.instance)}},
  });
}

Result<Reply> DefaultAppController::stop(const AppRequest& request) {
  auto& env = environment();

  if (!env.descriptor(request.appName).allowStop)
    return Err(ErrorCode::Forbidden, "application '{}' cannot be stopped", request.appName);

  if (auto running = findRunning(env, request); !running)
    return Err(running.error());

  if (auto stopped = env.launcher(request.appName).stop(request.appName); !stopped)
    return Err(stopped.error());

  return Ok(Reply{});
}

Result<Reply> DefaultAppController::hide(const AppRequest& request) {
  auto& env = environment();

  if (auto running = findRunning(env, request); !running)
    return Err(running.error());

  if (auto hidden = env.launcher(request.appName).hide(request.appName); !hidden)
    return Err(hidden.error());

  env.state(request.appName).state = RunState::Hidden;

  return Ok(Reply{});
}

Result<Reply> DefaultAppController::additionalData(const AppRequest& request) {
  environment().state(request.appName).additionalData = request.additionalData;

  return Ok(Reply{});
}

void DefaultAppController::onExit(std::string_view appName, int exitCode) {
  environment().state(appName).state = RunState::Stopped;
  std::println("app: '{}' stopped (exit code {})", appName, exitCode);
}

}  // namespace dibeast
