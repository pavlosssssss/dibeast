#include "Router.h"

#include <boost/url/parse.hpp>

#include <optional>
#include <ranges>

namespace dibeast::service {

namespace urls = boost::urls;

namespace {

using Segments = std::vector<std::string_view>;

Segments splitPattern(std::string_view pattern) {
  Segments segments;

  for (auto part : std::views::split(pattern, '/'))
    if (!part.empty())
      segments.emplace_back(part.begin(), part.end());

  return segments;
}

Segments pathSegments(const urls::url_view& url) {
  Segments segments;

  for (auto segment : url.encoded_segments())
    if (!segment.empty())
      segments.emplace_back(segment);

  return segments;
}

bool isCapture(std::string_view segment) {
  return segment.size() > 2 && segment.front() == '{' && segment.back() == '}';
}

std::optional<Segments> match(const std::vector<std::string>& pattern, const Segments& path) {
  if (pattern.size() != path.size())
    return std::nullopt;

  Segments captures;

  for (auto [expected, actual] : std::views::zip(pattern, path)) {
    if (isCapture(expected))
      captures.push_back(actual);
    else if (expected != actual)
      return std::nullopt;
  }

  return captures;
}

}  // namespace

void Router::add(boost::beast::http::verb method, std::string_view pattern, Handler handler) {
  std::vector<std::string> segments;

  for (auto segment : splitPattern(pattern))
    segments.emplace_back(segment);

  routes_.push_back({method, std::move(segments), std::move(handler)});
}

void Router::route(const Request& request, const RequestContext& context, Respond respond) const {
  const std::string_view target = request.target();
  const auto url = urls::parse_origin_form(target);

  if (!url)
    return respond(Err(ErrorCode::BadRequest, "malformed target {}", target));

  const auto path = pathSegments(*url);
  bool pathMatched = false;

  for (const auto& route : routes_) {
    auto captures = match(route.segments, path);

    if (!captures)
      continue;

    pathMatched = true;

    if (route.method == request.method())
      return route.handler(request, context, {std::move(*captures), url->params()},
                           std::move(respond));
  }

  if (pathMatched)
    return respond(Err(ErrorCode::MethodNotAllowed, "{} not allowed on {}",
                       std::string_view{request.method_string()}, target));

  respond(Err(ErrorCode::NotFound, "{} not found", target));
}

}  // namespace dibeast::service
