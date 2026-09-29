#pragma once

#include <concepts>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace dibeast {

enum class ErrorCode {
  BadRequest,
  Forbidden,
  NotFound,
  MethodNotAllowed,
  PayloadTooLarge,
  NotImplemented,
  ServiceUnavailable,
  Internal,
};

using Headers = std::vector<std::pair<std::string, std::string>>;

/**
 @brief Common error type for all layers; headers are added to the HTTP error response.
 **/
struct Error {
  ErrorCode code{};
  std::string message{};
  Headers headers{};
};

template <typename T>
using Result = std::expected<T, Error>;

using VoidResult = Result<void>;

inline VoidResult Ok() {
  return {};
}

template <typename T>
Result<std::decay_t<T>> Ok(T&& value) {
  return std::forward<T>(value);
}

inline std::unexpected<Error> Err(ErrorCode code, std::string message) {
  return std::unexpected(Error{code, std::move(message)});
}

template <typename... Args>
  requires(sizeof...(Args) > 0)
std::unexpected<Error> Err(ErrorCode code, std::format_string<Args...> format, Args&&... args) {
  return std::unexpected(Error{code, std::format(format, std::forward<Args>(args)...)});
}

template <typename E>
  requires std::same_as<std::remove_cvref_t<E>, Error>
std::unexpected<Error> Err(E&& error) {
  return std::unexpected<Error>(std::forward<E>(error));
}

inline std::string_view toString(ErrorCode code) {
  switch (code) {
    case ErrorCode::BadRequest:
      return "BadRequest";
    case ErrorCode::Forbidden:
      return "Forbidden";
    case ErrorCode::NotFound:
      return "NotFound";
    case ErrorCode::MethodNotAllowed:
      return "MethodNotAllowed";
    case ErrorCode::PayloadTooLarge:
      return "PayloadTooLarge";
    case ErrorCode::NotImplemented:
      return "NotImplemented";
    case ErrorCode::ServiceUnavailable:
      return "ServiceUnavailable";
    case ErrorCode::Internal:
      return "Internal";
  }

  return "Unknown";
}

}  // namespace dibeast
