#pragma once

#include "dibeast/Result.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>

namespace dibeast::service {

class Router;

/**
 @brief Accepts TCP connections and starts an HttpSession for each.
 **/
class HttpServer {
 public:
  HttpServer(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint endpoint,
             const Router& router);

  VoidResult start();

 private:
  void doAccept();
  void onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

  boost::asio::io_context& io_;
  boost::asio::ip::tcp::endpoint endpoint_;
  boost::asio::ip::tcp::acceptor acceptor_;
  const Router& router_;
};

}  // namespace dibeast::service
