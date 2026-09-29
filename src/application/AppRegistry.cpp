#include "AppRegistry.h"

#include <boost/url/parse.hpp>

namespace dibeast::app {

VoidResult AppRegistry::add(AppDescriptor descriptor, std::unique_ptr<AppLauncher> launcher) {
  if (descriptor.name.empty())
    return Err(ErrorCode::BadRequest, "application name must not be empty");

  if (!launcher)
    return Err(ErrorCode::BadRequest, "application '{}' has no launcher", descriptor.name);

  if (apps_.contains(descriptor.name))
    return Err(ErrorCode::BadRequest, "application '{}' is already registered", descriptor.name);

  std::vector<boost::urls::url> origins;

  for (const auto& origin : descriptor.allowedOrigins) {
    auto parsed = boost::urls::parse_uri(origin);

    if (!parsed)
      return Err(ErrorCode::BadRequest, "application '{}': invalid allowed origin '{}'",
                 descriptor.name, origin);

    origins.emplace_back(*parsed);
  }

  auto name = descriptor.name;
  apps_.emplace(std::move(name),
                RegisteredApp{std::move(descriptor), std::move(launcher), std::move(origins)});

  return Ok();
}

Result<const RegisteredApp*> AppRegistry::find(std::string_view name) const {
  if (auto it = apps_.find(name); it != apps_.end())
    return Ok(&it->second);

  return Err(ErrorCode::NotFound, "application '{}' not found", name);
}

}  // namespace dibeast::app
