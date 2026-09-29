#pragma once

#include "dibeast/Result.h"
#include "dibeast/Types.h"
#include "service/rest/RequestContext.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <memory>
#include <optional>

namespace dibeast::service {

class Router;

/**
 @brief One HTTP connection: read request, write response, repeat while keep-alive.
 **/
class HttpSession : public std::enable_shared_from_this<HttpSession> {
 public:
  HttpSession(boost::asio::ip::tcp::socket&& socket, const Router& router);

  void start();

 private:
  void doRead();
  void onRead(boost::beast::error_code ec, std::size_t bytes);
  void doWrite(boost::beast::http::message_generator&& response);
  void onWrite(bool keepAlive, boost::beast::error_code ec, std::size_t bytes);
  void doClose();

  void handle(const Request& request);
  void onHandled(Result<Reply> result);

  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;
  std::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;
  const Router& router_;
  RequestContext context_;
};

}  // namespace dibeast::service
