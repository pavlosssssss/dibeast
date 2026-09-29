#pragma once

#include "dibeast/Result.h"
#include "dibeast/Types.h"

#include <boost/beast/http.hpp>

namespace dibeast::service {

boost::beast::http::message_generator makeResponse(Result<Reply> result, unsigned version,
                                                   bool keepAlive);

}  // namespace dibeast::service
