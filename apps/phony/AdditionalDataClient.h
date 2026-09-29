#pragma once

#include "dibeast/Result.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dibeast::apps {

using FormData = std::vector<std::pair<std::string, std::string>>;

VoidResult postAdditionalData(std::string_view url, const FormData& data);

}  // namespace dibeast::apps
