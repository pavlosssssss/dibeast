# Deployment View

```plantuml
@startuml
node "First-screen device (Linux)" {
  component "Integrator's server process" as host {
    component "libdibeast.so.0" as lib #lightblue
  }
  component "First-screen app\n(child process or anything\nthe launcher starts)" as app #lightgreen
}
node "Second-screen device" {
  component "DIAL client" as client
}

client --> lib : UDP 1900 multicast (SSDP)
client --> lib : TCP httpPort (REST)
lib --> app : AppLauncher
app --> lib : TCP 127.0.0.1:httpPort (dial_data)
@enduml
```

| Element | Notes |
|---|---|
| `libdibeast.so.0` | Loaded by the integrator's process; one `DialApp` per process |
| Network | UDP 1900 on `239.255.255.250` (IPv4), TCP `Config::httpPort` (default 56789) on all interfaces |
| First-screen apps | Started by their launcher; the built-in one runs a child process that dies with the server |
