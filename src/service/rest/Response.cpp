#include "Response.h"

#include "common/Constants.h"

namespace dibeast::service {

namespace http = boost::beast::http;

namespace {

http::status toHttpStatus(Reply::Status status) {
  switch (status) {
    case Reply::Status::Ok:
      return http::status::ok;
    case Reply::Status::Created:
      return http::status::created;
  }

  return http::status::internal_server_error;
}

http::status toHttpStatus(ErrorCode code) {
  switch (code) {
    case ErrorCode::BadRequest:
      return http::status::bad_request;
    case ErrorCode::Forbidden:
      return http::status::forbidden;
    case ErrorCode::NotFound:
      return http::status::not_found;
    case ErrorCode::MethodNotAllowed:
      return http::status::method_not_allowed;
    case ErrorCode::PayloadTooLarge:
      return http::status::payload_too_large;
    case ErrorCode::NotImplemented:
      return http::status::not_implemented;
    case ErrorCode::ServiceUnavailable:
      return http::status::service_unavailable;
    case ErrorCode::Internal:
      return http::status::internal_server_error;
  }

  return http::status::internal_server_error;
}

}  // namespace

http::message_generator makeResponse(Result<Reply> result, unsigned version, bool keepAlive) {
  http::response<http::string_body> response;
  response.version(version);
  response.set(http::field::server, kServerName);
  response.keep_alive(keepAlive);

  if (result) {
    response.result(toHttpStatus(result->status));

    if (!result->contentType.empty())
      response.set(http::field::content_type, result->contentType);

    for (auto& [name, value] : result->headers)
      response.set(name, std::move(value));

    response.body() = std::move(result->body);
  } else {
    response.result(toHttpStatus(result.error().code));
    response.set(http::field::content_type, R"(text/plain; charset="utf-8")");
    for (auto& [name, value] : result.error().headers)
      response.set(name, std::move(value));

    response.body() = std::move(result.error().message) + "\n";
  }

  response.prepare_payload();

  return response;
}

}  // namespace dibeast::service
