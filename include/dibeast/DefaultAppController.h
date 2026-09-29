#pragma once

#include "dibeast/AppController.h"
#include "dibeast/Export.h"

#include <memory>

namespace dibeast {

/**
 @brief DIAL 2.2.1 behaviour of the REST service; subclass it to adjust single operations.
 **/
class DIBEAST_EXPORT DefaultAppController : public AppController {
 public:
  DefaultAppController();
  ~DefaultAppController() override;

  void attach(AppEnvironment& environment) override;

  Result<Reply> info(const AppRequest& request) override;
  Result<Reply> launch(const AppRequest& request) override;
  Result<Reply> stop(const AppRequest& request) override;
  Result<Reply> hide(const AppRequest& request) override;
  Result<Reply> additionalData(const AppRequest& request) override;
  void onExit(std::string_view appName, int exitCode) override;

 protected:
  AppEnvironment& environment();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace dibeast
