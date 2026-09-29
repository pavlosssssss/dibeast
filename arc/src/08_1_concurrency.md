# Concurrency

One `io_context` is run by `Config::threads` threads (0 = `max(2, hardware_concurrency)`).
Work is serialized by strands instead of mutexes:

| Strand | Serializes |
|---|---|
| One per HTTP session | That connection's reads, writes and timers |
| SSDP strand | The multicast socket and reply timers |
| App strand | Every `AppController` call and all app state ([ADR-3](09_architecture_decisions.md)) |

Results cross strands only by posting: the dispatcher posts controller calls to the app strand, and
their results are posted back to the session strand.

## Contract for integrator code

| Who | Rule |
|---|---|
| `DialApp` | Configure before `exec()`. `exec()` blocks until SIGINT/SIGTERM or `stop()`; `stop()` may be called from any thread |
| `AppController` | Every call, `onExit` included, runs on the app strand: no locks needed; must not block |
| `AppEnvironment` | Only valid inside controller calls |
| `AppLauncher` | `launch` / `stop` / `hide` run on the app strand. The `ExitCallback` may be called from any thread; the library posts it to the app strand and ignores it once `DialApp` is gone. Owned and destroyed by `DialApp` ([ADR-5](09_architecture_decisions.md)) |
