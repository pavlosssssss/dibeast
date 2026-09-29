#pragma once

#include "dibeast/Types.h"

#include <string>

namespace dibeast::app {

std::string renderServiceXml(const AppDescriptor& app, const AppState& state, RunState shownState);

}  // namespace dibeast::app
