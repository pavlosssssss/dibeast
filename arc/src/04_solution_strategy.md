# Solution Strategy

| Goal | Approach | Details |
|---|---|---|
| Stable ABI | Shared library with a small facade (`DialApp`) and abstract extension points; everything else internal | [Library API and ABI](08_4_library_api_and_abi.md), [ADR-4](09_architecture_decisions.md) |
| Extensibility | Per app an `AppLauncher`, globally one `AppController`; routes are generic on `{name}` | [Integrator Guide](a1_integrator_guide.md) |
| Simplicity | Three layers: **Service** (HTTP, SSDP) -> **Application** (DIAL rules) -> **Data** (state); dependencies point down only | [Building Block View](05_building_block_view.md) |
| Learnability | Plain Asio completion handlers, one strand for all app state, no locks in user code | [Concurrency](08_1_concurrency.md), [ADR-1, ADR-3](09_architecture_decisions.md) |
| Robustness | No exceptions; every fallible call returns `Result<T>` | [Error Handling](08_2_error_handling.md), [ADR-2](09_architecture_decisions.md) |
| Security | CORS and `dial_data` rules enforced by the library, not by replaceable controllers | [Security](08_3_security.md), [ADR-6](09_architecture_decisions.md) |
| Simplicity | In-memory state only | - |
