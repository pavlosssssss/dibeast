#pragma once

#include "dibeast/Result.h"

#include <boost/system/error_code.hpp>

#include <string_view>

namespace dibeast {

inline VoidResult toResult(const boost::system::error_code& ec, std::string_view what,
                           ErrorCode code = ErrorCode::Internal) {
  if (ec)
    return Err(code, "{}: {}", what, ec.message());

  return Ok();
}

}  // namespace dibeast
