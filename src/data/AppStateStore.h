#pragma once

#include "dibeast/Types.h"

#include <map>
#include <string>
#include <string_view>

namespace dibeast::data {

/**
 @brief In-memory application states; not thread-safe, used from the application strand only.
 **/
class AppStateStore {
 public:
  AppState& state(std::string_view name);

 private:
  std::map<std::string, AppState, std::less<>> states_;
};

}  // namespace dibeast::data
