#include "Routes.h"

#include "common/Url.h"

#include <boost/url/grammar/alnum_chars.hpp>
#include <boost/url/parse_query.hpp>

#include <algorithm>
#include <format>

namespace dibeast::service {

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace urls = boost::urls;

namespace {

bool isValidKey(std::string_view key) {
  return !key.empty() && std::ranges::all_of(key, urls::grammar::alnum_chars);
}

Result<AdditionalData> decodeForm(std::string_view body) {
  AdditionalData pairs;

  if (body.empty())
    return Ok(std::move(pairs));

  auto params = urls::parse_query(body);

  if (!params)
    return Err(ErrorCode::BadRequest, "malformed form body: {}", params.error().message());

  const urls::encoding_opts form{true};

  for (auto param : *params) {
    auto key = param.key.decode(form);

    if (!isValidKey(key))
      return Err(ErrorCode::BadRequest, "invalid additional data key '{}'", key);

    pairs.emplace_back(std::move(key), param.value.decode(form));
  }

  return Ok(std::move(pairs));
}

std::string queryParam(const urls::params_view& query, std::string_view key) {
  auto it = query.find(key);

  return it != query.end() ? (*it).value : std::string{};
}

AppRequest appRequest(const RequestContext& context, const Router::Match& match) {
  return {
      .appName = std::string{match.captures[0]},
      .instance = match.captures.size() > 1 ? std::string{match.captures[1]} : std::string{},
      .clientDialVersion = queryParam(match.query, "clientDialVer"),
      .applicationUrl = context.applicationUrl,
      .fromLocalhost = context.remote.address().is_loopback(),
  };
}

// DIAL 6.6: the origin is checked per app and, once allowed, echoed on every reply including
// errors.
Router::Handler withCors(const app::CorsPolicy& cors, Router::Handler handler) {
  return [&cors, handler = std::move(handler)](
             const Request& request, const RequestContext& context, const Router::Match& match,
             Router::Respond respond) {
    const std::string_view origin = request[http::field::origin];

    if (origin.empty())
      return handler(request, context, match, std::move(respond));

    auto allowed = cors.allowOrigin(match.captures[0], origin);

    if (!allowed)
      return respond(Err(allowed.error()));

    if (!*allowed)
      return handler(request, context, match, std::move(respond));

    handler(
        request, context, match,
        [origin = std::string{origin}, respond = std::move(respond)](Result<Reply> result) mutable {
          auto& headers = result ? result->headers : result.error().headers;
          headers.emplace_back("Access-Control-Allow-Origin", std::move(origin));
          respond(std::move(result));
        });
  };
}

Router::Handler forward(app::AppDispatcher& apps, app::AppDispatcher::Operation operation) {
  return [&apps, operation](const Request&, const RequestContext& context,
                            const Router::Match& match, Router::Respond respond) {
    apps.dispatch(operation, appRequest(context, match), std::move(respond));
  };
}

}  // namespace

void registerRoutes(Router& router, const app::DeviceController& device, app::AppDispatcher& apps,
                    const app::CorsPolicy& cors) {
  router.add(http::verb::get, "/dd.xml",
             [&device](const Request&, const RequestContext& context, const Router::Match&,
                       Router::Respond respond) {
               respond(device.deviceDescription(context.applicationUrl));
             });

  router.add(http::verb::get, "/apps/{name}", withCors(cors, forward(apps, &AppController::info)));

  router.add(http::verb::post, "/apps/{name}",
             withCors(cors, [&apps](const Request& request, const RequestContext& context,
                                    const Router::Match& match, Router::Respond respond) {
               auto call = appRequest(context, match);
               call.payload = request.body();
               call.additionalDataUrl =
                   httpUrl(asio::ip::address_v4::loopback(), context.local.port(),
                           std::format("/apps/{}/dial_data", call.appName));
               apps.dispatch(&AppController::launch, std::move(call), std::move(respond));
             }));

  router.add(http::verb::delete_, "/apps/{name}/{instance}",
             withCors(cors, forward(apps, &AppController::stop)));

  router.add(http::verb::post, "/apps/{name}/{instance}/hide",
             withCors(cors, forward(apps, &AppController::hide)));

  router.add(http::verb::post, "/apps/{name}/dial_data",
             withCors(cors, [&apps](const Request& request, const RequestContext& context,
                                    const Router::Match& match, Router::Respond respond) {
               auto call = appRequest(context, match);

               if (!call.fromLocalhost)
                 return respond(
                     Err(ErrorCode::Forbidden, "additional data is accepted from localhost only"));

               auto data = decodeForm(request.body());

               if (!data)
                 return respond(Err(data.error()));

               call.additionalData = std::move(*data);
               apps.dispatch(&AppController::additionalData, std::move(call), std::move(respond));
             }));
}

}  // namespace dibeast::service
