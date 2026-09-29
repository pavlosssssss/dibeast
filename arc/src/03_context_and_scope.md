# Context and Scope

```plantuml
@startuml
left to right direction

actor "DIAL client\n(second screen)" as client
actor "Integrator" as dev
node "First-screen device" {
  component "Integrator's server\n(e.g. examples/dial_server)" as host
  component "libdibeast" as server #lightblue
  component "First-screen app\n(e.g. phony child process)" as phony #lightgreen
}

dev ..> host : writes: config, apps,\nlaunchers, controller
host --> server : DialApp public API
client --> server : SSDP M-SEARCH
client --> server : HTTP REST\nGET /dd.xml, /apps/{name}
server --> phony : launch / stop / hide\nvia AppLauncher
phony --> server : POST /apps/{name}/dial_data\n(localhost)
@enduml
```

| Partner | Interface | Direction |
|---|---|---|
| Integrator | C++ public API of `libdibeast` ([Library API and ABI](08_4_library_api_and_abi.md)) | in |
| DIAL client | SSDP over UDP multicast (M-SEARCH request, unicast response) | in / out |
| DIAL client | HTTP/1.0 + 1.1 REST: device description and application resources | in / out |
| First-screen app | Whatever its `AppLauncher` does | out |
| First-screen app | HTTP `POST` of `x-www-form-urlencoded` additional data, localhost only | in |
