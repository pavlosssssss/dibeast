#include "phony/AdditionalDataClient.h"

#include <charconv>
#include <chrono>
#include <format>
#include <optional>
#include <print>
#include <string_view>
#include <thread>
#include <unistd.h>

namespace {

struct Options {
  std::string_view payload;
  std::string_view additionalDataUrl;
};

Options parse(int argc, char* argv[]) {
  Options options;

  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string_view key = argv[i];

    if (key == "--dial-payload")
      options.payload = argv[i + 1];
    else if (key == "--additional-data-url")
      options.additionalDataUrl = argv[i + 1];
  }

  return options;
}

std::optional<int> exitAfterSeconds(std::string_view payload) {
  constexpr std::string_view kKey = "exitAfter=";
  const auto pos = payload.find(kKey);

  if (pos == std::string_view::npos)
    return std::nullopt;

  const auto value = payload.substr(pos + kKey.size());
  int seconds = 0;

  if (std::from_chars(value.data(), value.data() + value.size(), seconds).ec != std::errc{})
    return std::nullopt;

  return seconds;
}

}  // namespace

int main(int argc, char* argv[]) {
  const auto options = parse(argc, argv);
  std::println("phony[{}]: payload='{}' additionalDataUrl='{}'", getpid(), options.payload,
               options.additionalDataUrl);

  if (!options.additionalDataUrl.empty()) {
    const dibeast::apps::FormData data{
        {"screenId", std::format("phony-{}", getpid())},
        {"payload", std::string{options.payload}},
    };

    if (auto posted = dibeast::apps::postAdditionalData(options.additionalDataUrl, data); !posted)
      std::println(stderr, "phony[{}]: additional data not sent: {}", getpid(),
                   posted.error().message);
  }

  if (auto seconds = exitAfterSeconds(options.payload)) {
    std::this_thread::sleep_for(std::chrono::seconds{*seconds});
    std::println("phony[{}]: exiting on my own", getpid());

    return 0;
  }

  pause();

  return 0;
}
