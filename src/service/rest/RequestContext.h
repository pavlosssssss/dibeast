#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

#include <string>

namespace dibeast::service {

using Request = boost::beast::http::request<boost::beast::http::string_body>;

/**
 @brief Connection facts a handler may need besides the request itself.
 **/
struct RequestContext {
  boost::asio::ip::tcp::endpoint local;
  boost::asio::ip::tcp::endpoint remote;
  std::string applicationUrl;
};

}  // namespace dibeast::service
