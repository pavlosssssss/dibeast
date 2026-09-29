#pragma once

#include <boost/asio/ip/address.hpp>

#include <format>
#include <string>
#include <string_view>

namespace dibeast {

inline std::string httpUrl(const boost::asio::ip::address& host, unsigned short port,
                           std::string_view path) {
  const auto address = host.to_string();

  return host.is_v6() ? std::format("http://[{}]:{}{}", address, port, path)
                      : std::format("http://{}:{}{}", address, port, path);
}

}  // namespace dibeast
