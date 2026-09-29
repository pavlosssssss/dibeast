#pragma once

#include "common/Strand.h"
#include "dibeast/Result.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>

#include <array>
#include <chrono>
#include <random>
#include <string>

namespace dibeast::service {

struct SsdpConfig {
  std::string uuid;
  unsigned short httpPort;
};

/**
 @brief Answers SSDP M-SEARCH requests for the DIAL service type.
 **/
class SsdpService {
 public:
  SsdpService(boost::asio::io_context& io, SsdpConfig config);

  VoidResult start();

 private:
  void doReceive();
  void onReceive(boost::system::error_code ec, std::size_t bytes);
  void scheduleReply(boost::asio::ip::udp::endpoint to, std::chrono::milliseconds delay);
  void sendReply(const boost::asio::ip::udp::endpoint& to);
  Result<boost::asio::ip::address> localAddressFor(const boost::asio::ip::udp::endpoint& peer);

  SsdpConfig config_;
  IoStrand strand_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_;
  std::array<char, 1500> buffer_{};
  std::mt19937 random_;
};

}  // namespace dibeast::service
