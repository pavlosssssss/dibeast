# Risks and Technical Debt

| Item | Impact | Mitigation |
|---|---|---|
| No automated tests yet | Regressions found only by manual curl / SSDP runs | Planned |
| A blocking launcher or controller stalls all apps | Single app strand ([ADR-3](09_architecture_decisions.md)) | Documented contract: must not block |
| SSDP is IPv4 only | No discovery on IPv6-only networks | Out of scope for now |
| ABI tied to one toolchain | Clients built with another compiler or standard library break | Documented in [Library API and ABI](08_4_library_api_and_abi.md) |
