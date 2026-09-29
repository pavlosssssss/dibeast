#include "DeviceController.h"

#include "application/Xml.h"
#include "common/Constants.h"

#include <format>

namespace dibeast::app {

namespace {

constexpr std::string_view kDescriptionTemplate = R"(<?xml version="1.0" encoding="UTF-8"?>
<root xmlns="urn:schemas-upnp-org:device-1-0">
  <specVersion>
    <major>1</major>
    <minor>0</minor>
  </specVersion>
  <device>
    <deviceType>urn:dial-multiscreen-org:device:dial:1</deviceType>
    <friendlyName>{}</friendlyName>
    <manufacturer>{}</manufacturer>
    <modelName>{}</modelName>
    <UDN>uuid:{}</UDN>
  </device>
</root>
)";

}  // namespace

DeviceController::DeviceController(const DeviceInfo& device)
    : description_{std::format(kDescriptionTemplate, xmlEscape(device.friendlyName),
                               xmlEscape(device.manufacturer), xmlEscape(device.modelName),
                               xmlEscape(device.uuid))} {}

Reply DeviceController::deviceDescription(std::string applicationUrl) const {
  return {
      .status = Reply::Status::Ok,
      .contentType = std::string{kXmlContentType},
      .body = description_,
      .headers = {{"Application-URL", std::move(applicationUrl)}},
  };
}

}  // namespace dibeast::app
