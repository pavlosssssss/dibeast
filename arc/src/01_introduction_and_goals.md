# Introduction and Goals

A DIAL 2.2.1 server ([spec](../../DIAL-2ndScreenProtocol-2.2.1.txt)) written as a pet project to
learn **Boost.Beast** and keep practicing **Boost.Asio**. A second-screen device (phone) discovers
the server via SSDP and then queries, launches and stops first-screen applications via REST.

The server is the shared library **`libdibeast`** ("DIAL on Beast"). An integrator registers apps,
plugs in how each app is started (`AppLauncher`) and, if needed, replaces the DIAL behaviour
(`AppController`). `examples/dial_server` uses it with three sample apps, one per kind of launcher.

## Scope

| Spec feature | Section | Status |
|---|---|---|
| SSDP M-SEARCH response | §5.1, §5.2 | in scope |
| Device description with `Application-URL` header | §5.3, §5.4 | in scope |
| Query application info (XML) | §6.1 | in scope |
| Launch application (201 / 404 / 413 / 503) | §6.2 | in scope |
| Additional data from first-screen app (`dial_data`) | §6.3 | in scope |
| Stop application | §6.4 | in scope |
| CORS origin check | §6.6 | in scope |
| Hide application | §6.5 | delegated to the app's launcher |
| `hidden` state only for `clientDialVer` >= 2.1 | §6.1.2 | in scope |
| `installable=` state, HDMI-CEC, 403 client authorization | §6.1.2, §6.2.2 | out of scope |
| Wake-on-LAN (`WAKEUP` header) | §7 | out of scope |
| System app / low power mode | §8 | out of scope |

## Quality goals

| Goal | Meaning |
|---|---|
| Learnability | Code reads as a study guide for Beast/Asio: one idea per class, no hidden magic |
| Extensibility | New apps, launchers and controller behaviour without touching the library |
| Stable ABI | Clients link against a small, versioned symbol set |
| Simplicity | No framework on top of Beast; no abstraction without a second user |
