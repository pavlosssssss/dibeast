# Security

All rules below are enforced by the library before a controller runs, so a custom `AppController`
cannot skip them ([ADR-6](09_architecture_decisions.md)).

| Rule | Spec | Behaviour |
|---|---|---|
| CORS origin check on `/apps/{name}...` | §6.6 | Requests without `Origin` pass. `http`, `file`, `ftp` or malformed origins -> 403. `https` origins match an allowed host, optionally `*.domain` for one subdomain level; other schemes match exactly. An allowed origin is echoed in `Access-Control-Allow-Origin` on every reply, errors included |
| Allowed origins are validated | §6.6 | Parsed when the app is registered; a malformed pattern makes `addApp` fail |
| `dial_data` only from the device itself | §6.3 | Non-localhost clients -> 403 |
| `dial_data` keys | §6.3 | Only `[0-9a-zA-Z]`, else 400 |
| Request body limit | §6.2 | 4 KB, larger bodies -> 413 |
| Payload never becomes a command line | §6.2.1 | The built-in launcher passes payload and `additionalDataUrl` as separate arguments (`--dial-payload`, `--additional-data-url`) |
