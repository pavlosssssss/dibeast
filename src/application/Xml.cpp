#include "Xml.h"

namespace dibeast::app {

std::string xmlEscape(std::string_view text) {
  std::string out;
  out.reserve(text.size());

  for (char c : text) {
    switch (c) {
      case '&':
        out += "&amp;";
        break;
      case '<':
        out += "&lt;";
        break;
      case '>':
        out += "&gt;";
        break;
      case '"':
        out += "&quot;";
        break;
      case '\'':
        out += "&apos;";
        break;
      default:
        out += c;
    }
  }

  return out;
}

}  // namespace dibeast::app
