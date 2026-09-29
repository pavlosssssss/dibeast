#pragma once

#include "dibeast/Types.h"

namespace dibeast::apps {

inline AppDescriptor makePhonyDescriptor() {
  return {
      .name = "phony",
      .allowStop = true,
      .allowedOrigins = {"https://phony.example.com", "https://*.phony.dev",
                         "package:com.example.phony"},
  };
}

}  // namespace dibeast::apps
