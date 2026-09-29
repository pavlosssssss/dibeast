#include "AppStateStore.h"

namespace dibeast::data {

AppState& AppStateStore::state(std::string_view name) {
  if (auto it = states_.find(name); it != states_.end())
    return it->second;

  return states_.emplace(name, AppState{}).first->second;
}

}  // namespace dibeast::data
