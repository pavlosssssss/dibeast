# Quality Requirements

| Goal | Scenario | Measure |
|---|---|---|
| Extensibility | An integrator adds an app started by a custom mechanism | Only a descriptor and an `AppLauncher` subclass; no library change, no new route |
| Stable ABI | A patch release of the library is dropped in | Clients keep working without recompiling; exported symbol set unchanged |
| Robustness | An app exits on its own, or the server stops while apps run | Reported state becomes `stopped`; shutdown leaves no child processes and no late callbacks |
| Learnability | A reader looks for how one request is handled | One class per step (session, router, routes, dispatcher, controller, response) |
