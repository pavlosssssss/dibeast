#pragma once

#include "dibeast/Types.h"

#include <string>

namespace dibeast::app {

/**
 @brief Serves the UPnP device description with the DIAL Application-URL header.
 **/
class DeviceController {
 public:
  explicit DeviceController(const DeviceInfo& device);

  Reply deviceDescription(std::string applicationUrl) const;

 private:
  std::string description_;
};

}  // namespace dibeast::app
