#include "SsdpService.h"

#include "common/Constants.h"
#include "common/SystemError.h"
#include "common/Url.h"

#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <charconv>
#include <format>
#include <memory>
#include <optional>
#include <print>

namespace dibeast::service {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using udp = asio::ip::udp;

namespace {

const auto kMulticastAddress = asio::ip::make_address_v4("239.255.255.250");
constexpr unsigned short kSsdpPort = 1900;
constexpr std::string_view kDialSearchTarget = "urn:dial-multiscreen-org:service:dial:1";
constexpr int kMaxWaitLimitSeconds = 5;

struct SearchRequest {
  std::string searchTarget;
  int maxWaitSeconds;
};

std::optional<SearchRequest> parseSearch(std::string_view datagram) {
  http::request_parser<http::empty_body> parser;
  beast::error_code ec;
  parser.put(asio::buffer(datagram.data(), datagram.size()), ec);

  if (ec || !parser.is_header_done())
    return std::nullopt;

  const auto& request = parser.get();

  if (request.method() != http::verb::msearch || request["MAN"] != R"("ssdp:discover")")
    return std::nullopt;

  int maxWait = 1;
  const auto mx = request["MX"];
  std::from_chars(mx.data(), mx.data() + mx.size(), maxWait);

  return SearchRequest{std::string{request["ST"]}, std::clamp(maxWait, 1, kMaxWaitLimitSeconds)};
}

bool isDialSearch(std::string_view searchTarget) {
  return searchTarget == kDialSearchTarget || searchTarget == "ssdp:all";
}

std::string makeSearchResponse(const SsdpConfig& config, const asio::ip::address& host) {
  return std::format(
      "HTTP/1.1 200 OK\r\n"
      "LOCATION: {}\r\n"
      "CACHE-CONTROL: max-age=1800\r\n"
      "EXT:\r\n"
      "BOOTID.UPNP.ORG: 1\r\n"
      "SERVER: Linux/1.0 UPnP/1.1 {}\r\n"
      "USN: uuid:{}::{}\r\n"
      "ST: {}\r\n"
      "\r\n",
      httpUrl(host, config.httpPort, "/dd.xml"), kServerName, config.uuid, kDialSearchTarget,
      kDialSearchTarget);
}

}  // namespace

SsdpService::SsdpService(asio::io_context& io, SsdpConfig config)
    : config_{std::move(config)}
    , strand_{asio::make_strand(io)}
    , socket_{strand_}
    , random_{std::random_device{}()} {}

VoidResult SsdpService::start() {
  boost::system::error_code ec;

  socket_.open(udp::v4(), ec);
  if (auto r = toResult(ec, "ssdp open"); !r)
    return r;

  socket_.set_option(asio::socket_base::reuse_address{true}, ec);
  if (auto r = toResult(ec, "ssdp reuse_address"); !r)
    return r;

  socket_.bind({asio::ip::address_v4::any(), kSsdpPort}, ec);
  if (auto r = toResult(ec, "ssdp bind"); !r)
    return r;

  socket_.set_option(asio::ip::multicast::join_group{kMulticastAddress}, ec);
  if (auto r = toResult(ec, "ssdp join_group"); !r)
    return r;

  std::println("ssdp: listening on {}:{}", kMulticastAddress.to_string(), kSsdpPort);
  doReceive();

  return Ok();
}

void SsdpService::doReceive() {
  socket_.async_receive_from(
      asio::buffer(buffer_), sender_,
      [this](boost::system::error_code ec, std::size_t bytes) { onReceive(ec, bytes); });
}

void SsdpService::onReceive(boost::system::error_code ec, std::size_t bytes) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  if (ec == asio::error::operation_aborted)
    return;

  if (ec) {
    std::println(stderr, "ssdp receive: {}", ec.message());
  } else if (auto search = parseSearch({buffer_.data(), bytes});
             search && isDialSearch(search->searchTarget)) {
    std::uniform_int_distribution<int> delayMs{0, search->maxWaitSeconds * 1000 - 1};
    const std::chrono::milliseconds delay{delayMs(random_)};
    std::println("ssdp: M-SEARCH from {}:{} ST '{}', reply in {}", sender_.address().to_string(),
                 sender_.port(), search->searchTarget, delay);
    scheduleReply(sender_, delay);
  }

  doReceive();
}

void SsdpService::scheduleReply(udp::endpoint to, std::chrono::milliseconds delay) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  auto timer = std::make_shared<asio::steady_timer>(strand_, delay);
  timer->async_wait([this, timer, to](boost::system::error_code ec) {
    if (!ec)
      sendReply(to);
  });
}

void SsdpService::sendReply(const udp::endpoint& to) {
  DIBEAST_ASSERT_ON_STRAND(strand_);

  auto host = localAddressFor(to);

  if (!host) {
    std::println(stderr, "{}", host.error().message);

    return;
  }

  auto message = std::make_shared<std::string>(makeSearchResponse(config_, *host));
  socket_.async_send_to(asio::buffer(*message), to,
                        [message](boost::system::error_code ec, std::size_t) {
                          if (ec)
                            std::println(stderr, "ssdp send: {}", ec.message());
                        });
}

Result<asio::ip::address> SsdpService::localAddressFor(const udp::endpoint& peer) {
  // Connecting a UDP socket sends nothing; it only makes the kernel pick the outgoing interface.
  udp::socket probe{strand_};
  boost::system::error_code ec;
  probe.connect(peer, ec);
  const auto local = probe.local_endpoint(ec);

  return toResult(ec, std::format("ssdp: no route to {}", peer.address().to_string()))
      .transform([&local] { return local.address(); });
}

}  // namespace dibeast::service
