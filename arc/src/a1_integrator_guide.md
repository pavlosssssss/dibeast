# Appendix: Integrator Guide - Adding a New App

Routes are generic on `{name}`, so a new app never needs new routes. Pick the smallest option that
fits; `examples/dial_server` shows each of them. Everything is configured before `exec()`; the
rules integrator code must follow are in [Concurrency](08_1_concurrency.md).

## Option 1 - An app that is a program

| Step | What to do |
|---|---|
| Describe it | An `AppDescriptor`: `name` (as registered in the DIAL registry, §9, case-sensitive), `allowStop`, `allowedOrigins` ([Security](08_3_security.md)) |
| Launcher | `DialApp::makeProcessLauncher(executable)`; the app gets payload and `additionalDataUrl` as arguments |
| Register | `DialApp::addApp(descriptor, launcher)`; rejects an empty or duplicate name, a missing launcher or a malformed allowed origin |

Worked example: `phony` (`apps/phony/`).

## Option 2 - An app started some other way

Implement `AppLauncher` (e.g. a browser app, a system service, an in-process app) and register it
with `addApp` as above.

| Method | Contract |
|---|---|
| `launch(request, onExit)` | Start the app with `request.payload`, hand it `request.additionalDataUrl`; `Err(ServiceUnavailable, ...)` if it cannot start (-> 503). Call `onExit` once when the app ends |
| `stop(appName)` | Ask the app to exit; the state becomes `stopped` when `onExit` is called |
| `hide(appName)` | Optional; return `Ok()` to support hide (state becomes `hidden`), default is 501 |

Worked examples: `DemoLauncher` (in-process), `ForkExecLauncher` (fork/exec, reports exits from its
own thread), both in `examples/dial_server/`.

## Option 3 - Different DIAL behaviour

Subclass `DefaultAppController` and override single operations (e.g. `launch` returning
`Err(Forbidden, ...)` for unauthorized clients, §6.2.2), calling the base class for the rest; or
implement `AppController` from scratch. Install it with `DialApp::setAppController`. The controller
reaches apps, state and launchers only through `AppEnvironment`.

Worked example: `LoggingAppController` (`examples/dial_server/`).
