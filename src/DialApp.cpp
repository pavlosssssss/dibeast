#include "dibeast/DialApp.h"

#include "application/AppDispatcher.h"
#include "application/AppRegistry.h"
#include "application/CorsPolicy.h"
#include "application/DefaultProcessLauncher.h"
#include "application/DeviceController.h"
#include "data/AppStateStore.h"
#include "service/rest/HttpServer.h"
#include "service/rest/Router.h"
#include "service/rest/Routes.h"
#include "service/ssdp/SsdpService.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>

#include <algorithm>
#include <exception>
#include <print>
#include <thread>
#include <vector>

namespace dibeast {

namespace asio = boost::asio;

namespace {

unsigned threadCount(unsigned requested) {
  return requested != 0 ? requested : std::max(2u, std::thread::hardware_concurrency());
}

}  // namespace

struct DialApp::Impl {
  explicit Impl(Config config)
      : config{std::move(config)}
      , threads{threadCount(this->config.threads)}
      , io{static_cast<int>(threads)}
      , signals{io, SIGINT, SIGTERM}
      , appStrand{asio::make_strand(io)}
      , deviceController{this->config.device}
      , dispatcher{appStrand, registry, store}
      , cors{registry}
      , httpServer{io, {asio::ip::address_v4::any(), this->config.httpPort}, router}
      , ssdp{io, {.uuid = this->config.device.uuid, .httpPort = this->config.httpPort}} {
    service::registerRoutes(router, deviceController, dispatcher, cors);
  }

  void runIo() {
    // Safety net: our code does not throw, but Boost/std may (e.g. bad_alloc).
    try {
      io.run();
    } catch (const std::exception& e) {
      std::println(stderr, "fatal: unhandled exception: {}", e.what());
      io.stop();
    }
  }

  Config config;
  unsigned threads;
  asio::io_context io;
  asio::signal_set signals;
  IoStrand appStrand;
  app::DeviceController deviceController;
  app::AppRegistry registry;
  data::AppStateStore store;
  app::AppDispatcher dispatcher;
  app::CorsPolicy cors;
  service::Router router;
  service::HttpServer httpServer;
  service::SsdpService ssdp;
};

DialApp::DialApp(Config config)
    : impl_{std::make_unique<Impl>(std::move(config))} {}

DialApp::~DialApp() = default;

std::unique_ptr<AppLauncher> DialApp::makeProcessLauncher(std::string executable) {
  return std::make_unique<app::DefaultProcessLauncher>(impl_->appStrand, std::move(executable));
}

VoidResult DialApp::setAppController(std::unique_ptr<AppController> controller) {
  if (!controller)
    return Err(ErrorCode::BadRequest, "app controller must not be null");

  impl_->dispatcher.setController(std::move(controller));

  return Ok();
}

VoidResult DialApp::addApp(AppDescriptor descriptor, std::unique_ptr<AppLauncher> launcher) {
  return impl_->registry.add(std::move(descriptor), std::move(launcher));
}

int DialApp::exec() {
  auto& impl = *impl_;
  impl.dispatcher.attach();

  impl.signals.async_wait([&impl](const boost::system::error_code& ec, int signal) {
    if (ec)
      return;

    std::println("signal {} received, stopping", signal);
    impl.io.stop();
  });

  if (auto started = impl.httpServer.start().and_then([&impl] { return impl.ssdp.start(); });
      !started) {
    std::println(stderr, "fatal: {}", started.error().message);

    return 1;
  }

  std::vector<std::jthread> workers;
  workers.reserve(impl.threads - 1);

  for (unsigned i = 1; i < impl.threads; ++i)
    workers.emplace_back([&impl] { impl.runIo(); });

  impl.runIo();

  std::println("dial server stopped");

  return 0;
}

void DialApp::stop() {
  impl_->io.stop();
}

}  // namespace dibeast
