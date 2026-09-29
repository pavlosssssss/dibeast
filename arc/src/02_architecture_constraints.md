# Architecture Constraints

| Constraint | Background |
|---|---|
| g++-14, C++23, CMake | Learning modern C++ (`std::expected`, `std::format`, `std::move_only_function`) |
| Boost Asio, Beast, URL, Process v2, fetched via `FetchContent` | Learning Beast is the point of the project; no system Boost needed |
| Completion handlers, no coroutines | See the Asio mechanics explicitly ([ADR-1](09_architecture_decisions.md)) |
| Linux only | Process handling (pidfd, fork/exec) and the ABI rules (ELF, version script) target Linux |
| Shared library with a stable ABI | Integrators link `libdibeast.so` ([ADR-4](09_architecture_decisions.md)) |
