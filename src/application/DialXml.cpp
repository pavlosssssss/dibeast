#include "DialXml.h"

#include "application/Xml.h"

#include <format>
#include <iterator>
#include <string_view>

namespace dibeast::app {

namespace {

constexpr std::string_view kDialVersion = "2.2";

std::string_view toString(RunState state) {
  switch (state) {
    case RunState::Stopped:
      return "stopped";
    case RunState::Starting:
    case RunState::Running:
      return "running";
    case RunState::Hidden:
      return "hidden";
  }

  return "stopped";
}

}  // namespace

std::string renderServiceXml(const AppDescriptor& app, const AppState& state, RunState shownState) {
  std::string xml;
  auto out = std::back_inserter(xml);

  std::format_to(out,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<service xmlns=\"urn:dial-multiscreen-org:schemas:dial\" dialVer=\"{}\">\n"
                 "  <name>{}</name>\n"
                 "  <options allowStop=\"{}\"/>\n"
                 "  <state>{}</state>\n",
                 kDialVersion, xmlEscape(app.name), app.allowStop, toString(shownState));

  if (isRunning(shownState) && app.allowStop)
    std::format_to(out, "  <link rel=\"run\" href=\"{}\"/>\n", xmlEscape(state.instance));

  if (!state.additionalData.empty()) {
    xml += "  <additionalData>\n";

    for (const auto& [key, value] : state.additionalData)
      std::format_to(out, "    <{0}>{1}</{0}>\n", key, xmlEscape(value));

    xml += "  </additionalData>\n";
  }

  xml += "</service>\n";

  return xml;
}

}  // namespace dibeast::app
