#include "CorsPolicy.h"

#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/parse.hpp>

#include <algorithm>

namespace dibeast::app {

namespace urls = boost::urls;

namespace {

bool isInsecure(urls::scheme scheme) {
  return scheme == urls::scheme::http || scheme == urls::scheme::file ||
         scheme == urls::scheme::ftp;
}

bool hostMatches(std::string_view pattern, std::string_view host) {
  constexpr std::string_view kWildcard = "*.";

  if (!pattern.starts_with(kWildcard))
    return urls::grammar::ci_is_equal(pattern, host);

  const auto domain = pattern.substr(kWildcard.size());

  if (host.size() <= domain.size() + 1)
    return false;

  const auto label = host.substr(0, host.size() - domain.size() - 1);

  return host[label.size()] == '.' && label.find('.') == std::string_view::npos &&
         urls::grammar::ci_is_equal(host.substr(label.size() + 1), domain);
}

bool originMatches(const urls::url_view& pattern, const urls::url_view& origin) {
  if (!urls::grammar::ci_is_equal(pattern.scheme(), origin.scheme()))
    return false;

  if (origin.scheme_id() == urls::scheme::https)
    return hostMatches(pattern.encoded_host(), origin.encoded_host());

  const auto rest = [](const urls::url_view& uri) {
    return uri.buffer().substr(uri.scheme().size() + 1);
  };

  return rest(pattern) == rest(origin);
}

}  // namespace

CorsPolicy::CorsPolicy(const AppRegistry& registry)
    : registry_{registry} {}

Result<bool> CorsPolicy::allowOrigin(std::string_view appName, std::string_view origin) const {
  auto app = registry_.find(appName);

  if (!app)
    return Ok(false);

  auto uri = urls::parse_uri(origin);

  if (!uri)
    return Err(ErrorCode::Forbidden, "malformed origin '{}'", origin);

  if (isInsecure(uri->scheme_id()))
    return Err(ErrorCode::Forbidden, "insecure origin '{}'", origin);

  const auto& allowed = (*app)->allowedOrigins;

  if (std::ranges::none_of(allowed, [&](const auto& a) { return originMatches(a, *uri); }))
    return Err(ErrorCode::Forbidden, "origin '{}' not authorized for '{}'", origin, appName);

  return Ok(true);
}

}  // namespace dibeast::app
