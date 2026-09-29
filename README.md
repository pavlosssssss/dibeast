# DiBeast

**DiBeast** is a vibe coded DIAL 2.2.1 Server Implementation. See [DIAL 2.2.1](DIAL-2ndScreenProtocol-2.2.1.txt). 
Here a server library written on Boost.Beast and Boost.Asio. 
You configure the API surface (apps and, optionally, your own DIAL controller) and
bind each app to a launcher that knows how to start it; DiBeast does SSDP discovery and the REST
service.

```cpp
#include <dibeast/DialApp.h>

int main() {
  dibeast::DialApp app{{.device = {.uuid = "...", .friendlyName = "My TV"}, .httpPort = 56789}};
  app.addApp({.name = "phony"}, app.makeProcessLauncher("/usr/bin/phony_app"));
  return app.exec();  // Ctrl+C to stop
}
```

Build (CMake >= 3.22, g++-14; Boost is downloaded on first configure):

```sh
cmake --preset debug
cmake --build --preset debug -j
./build/debug/examples/dibeast_server
```

More: [Integrator Guide](arc/src/a1_integrator_guide.md) (custom launchers and controllers),
[Library API and ABI](arc/src/08_4_library_api_and_abi.md), full architecture in [`arc/`](arc/src/SUMMARY.md)
(`cd arc && mdbook serve`).
