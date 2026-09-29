#include "AdditionalDataClient.h"

#include "common/SystemError.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/url/encode.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/rfc/unreserved_chars.hpp>

namespace dibeast::apps {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace urls = boost::urls;
using tcp = asio::ip::tcp;

namespace {

std::string encodeForm(const FormData& data) {
  const urls::encoding_opts form{true};
  std::string body;

  for (const auto& [key, value] : data) {
    if (!body.empty())
      body += '&';

    body += urls::encode(key, urls::unreserved_chars, form);
    body += '=';
    body += urls::encode(value, urls::unreserved_chars, form);
  }

  return body;
}

}  // namespace

VoidResult postAdditionalData(std::string_view url, const FormData& data) {
  auto uri = urls::parse_uri(url);

  if (!uri)
    return Err(ErrorCode::BadRequest, "bad url '{}': {}", url, uri.error().message());

  asio::io_context io;
  tcp::resolver resolver{io};
  beast::tcp_stream stream{io};
  boost::system::error_code ec;

  const auto endpoints = resolver.resolve(uri->host(), uri->port(), ec);

  if (auto r = toResult(ec, "resolve"); !r)
    return r;

  stream.connect(endpoints, ec);

  if (auto r = toResult(ec, "connect"); !r)
    return r;

  http::request<http::string_body> request{http::verb::post, uri->encoded_path(), 11};
  request.set(http::field::host, uri->encoded_host_and_port());
  request.set(http::field::content_type, R"(application/x-www-form-urlencoded; charset="utf-8")");
  request.body() = encodeForm(data);
  request.prepare_payload();

  http::write(stream, request, ec);

  if (auto r = toResult(ec, "write"); !r)
    return r;

  beast::flat_buffer buffer;
  http::response<http::string_body> response;
  http::read(stream, buffer, response, ec);

  if (auto r = toResult(ec, "read"); !r)
    return r;

  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  if (response.result() != http::status::ok)
    return Err(ErrorCode::Internal, "server replied {}: {}", response.result_int(),
               response.body());

  return Ok();
}

}  // namespace dibeast::apps
