#include "HttpSession.h"

#include "common/Strand.h"
#include "common/Url.h"
#include "service/rest/Response.h"
#include "service/rest/Router.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>

#include <chrono>
#include <print>

namespace dibeast::service {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using namespace std::chrono_literals;

namespace {

constexpr std::size_t kMaxBodySize = 4096;
constexpr auto kIdleTimeout = 30s;

RequestContext makeContext(const asio::ip::tcp::socket& socket) {
  boost::system::error_code ec;
  const auto local = socket.local_endpoint(ec);

  return {
      .local = local,
      .remote = socket.remote_endpoint(ec),
      .applicationUrl = httpUrl(local.address(), local.port(), "/apps"),
  };
}

}  // namespace

HttpSession::HttpSession(asio::ip::tcp::socket&& socket, const Router& router)
    : stream_{std::move(socket)}
    , router_{router}
    , context_{makeContext(stream_.socket())} {}

void HttpSession::start() {
  // The socket was accepted on its own strand; run the session there.
  asio::dispatch(stream_.get_executor(),
                 beast::bind_front_handler(&HttpSession::doRead, shared_from_this()));
}

void HttpSession::doRead() {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  parser_.emplace();
  parser_->body_limit(kMaxBodySize);
  stream_.expires_after(kIdleTimeout);

  http::async_read(stream_, buffer_, *parser_,
                   beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
}

void HttpSession::onRead(beast::error_code ec, std::size_t) {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  if (ec == http::error::end_of_stream)
    return doClose();

  if (ec == http::error::body_limit)
    return doWrite(makeResponse(
        Err(ErrorCode::PayloadTooLarge, "body exceeds {} bytes", kMaxBodySize), 11, false));

  if (ec) {
    if (ec != beast::error::timeout)
      std::println(stderr, "http read: {}", ec.message());

    return;
  }

  handle(parser_->get());
}

void HttpSession::doWrite(http::message_generator&& response) {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  const bool keepAlive = response.keep_alive();
  beast::async_write(
      stream_, std::move(response),
      beast::bind_front_handler(&HttpSession::onWrite, shared_from_this(), keepAlive));
}

void HttpSession::onWrite(bool keepAlive, beast::error_code ec, std::size_t) {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  if (ec) {
    std::println(stderr, "http write: {}", ec.message());

    return;
  }

  if (!keepAlive)
    return doClose();

  doRead();
}

void HttpSession::doClose() {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  beast::error_code ec;
  stream_.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
}

void HttpSession::handle(const Request& request) {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  router_.route(request, context_, [self = shared_from_this()](Result<Reply> result) {
    // Controllers may complete on their own strand; hop back to the session's strand.
    asio::post(self->stream_.get_executor(),
               beast::bind_front_handler(&HttpSession::onHandled, self, std::move(result)));
  });
}

void HttpSession::onHandled(Result<Reply> result) {
  DIBEAST_ASSERT_ON_STRAND(stream_.get_executor());

  const auto& request = parser_->get();
  std::println("http: {} {} -> {}", std::string_view{request.method_string()},
               std::string_view{request.target()}, result ? "ok" : toString(result.error().code));
  doWrite(makeResponse(std::move(result), request.version(), request.keep_alive()));
}

}  // namespace dibeast::service
