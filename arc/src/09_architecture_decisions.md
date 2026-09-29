# Architecture Decisions

| # | Decision | Why | Consequence |
|---|---|---|---|
| ADR-1 | Completion handlers, no coroutines | Learning goal: see Asio's handler and lifetime mechanics directly | Sessions keep themselves alive with `shared_from_this`; more small callbacks |
| ADR-2 | No exceptions; `Result<T>` (`std::expected`) everywhere | Error paths visible in signatures; errors cross layers unchanged | Boost/std still need exceptions enabled, so a safety net stays around `io_context::run()` |
| ADR-3 | One app strand for all app state and controller calls | No mutexes; integrator code needs no locks and no Asio | Controllers and launchers must not block; app operations run one at a time |
| ADR-4 | Shared library: hidden visibility, version script, pimpl, no Boost in public headers | Small, versioned ABI; Boost stays an implementation detail | Integrators must use the same toolchain (`std` types in the API) |
| ADR-5 | One `AppLauncher` per app, owned by `DialApp`; exit callbacks go through a guard | Launchers cannot outlive the `io_context` or report to a destroyed dispatcher | Integrators keep no handle to a launcher after `addApp` |
| ADR-6 | CORS, `dial_data` rules and unknown-app 404 are enforced by the library, not the controller | Protocol and security rules cannot be dropped by replacing the controller | Controllers are only called for registered apps with valid input |
| ADR-7 | Launching an already running app returns 201 + `LOCATION` | §6.2.2 is ambiguous (table: 200, text: 201); the reference implementation returns 201 | A client always gets the Application Instance URL |
| ADR-8 | Hide is delegated to the launcher (default 501) | Only the launcher knows how to hide its app | Hide works per app, where the launcher supports it |
