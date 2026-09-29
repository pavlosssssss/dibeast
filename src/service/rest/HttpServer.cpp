#include "HttpServer.h"

#include "HttpSession.h"
#include "common/Strand.h"
#include "common/SystemError.h"

#include <boost/asio/strand.hpp>

#include <memory>
#include <print>

namespace dibeast::service {

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

HttpServer::HttpServer(asio::io_context& io, tcp::endpoint endpoint, const Router& router)
    : io_{io}
    , endpoint_{std::move(endpoint)}
    , acceptor_{asio::make_strand(io)}
    , router_{router} {}

VoidResult HttpServer::start() {
  boost::system::error_code ec;
  acceptor_.open(endpoint_.protocol(), ec);

  if (auto r = toResult(ec, "http open"); !r)
    return r;

  acceptor_.set_option(asio::socket_base::reuse_address{true}, ec);

  if (auto r = toResult(ec, "http reuse_address"); !r)
    return r;

  acceptor_.bind(endpoint_, ec);

  if (auto r = toResult(ec, "http bind"); !r)
    return r;

  acceptor_.listen(asio::socket_base::max_listen_connections, ec);

  if (auto r = toResult(ec, "http listen"); !r)
    return r;

  std::println("http: listening on {}:{}", endpoint_.address().to_string(), endpoint_.port());
  doAccept();

  return Ok();
}

void HttpServer::doAccept() {
  acceptor_.async_accept(
      asio::make_strand(io_),
      [this](boost::beast::error_code ec, tcp::socket socket) { onAccept(ec, std::move(socket)); });
}

void HttpServer::onAccept(boost::beast::error_code ec, tcp::socket socket) {
  DIBEAST_ASSERT_ON_STRAND(acceptor_.get_executor());

  if (ec == asio::error::operation_aborted)
    return;

  if (ec)
    std::println(stderr, "http accept: {}", ec.message());
  else
    std::make_shared<HttpSession>(std::move(socket), router_)->start();

  doAccept();
}

}  // namespace dibeast::service
