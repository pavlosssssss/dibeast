# Runtime View

## Scenario 1: Discovery

```plantuml
@startuml
participant "DIAL Client" as C
participant "SsdpService\n[ssdp strand]" as SS #lightblue
participant "HttpSession" as HS #lightblue

C -> SS: M-SEARCH (multicast, ST = dial, MX)
SS -> SS: wait random 0..MX
SS --> C: unicast 200 OK, LOCATION http://<ip>:<port>/dd.xml
C -> HS: GET /dd.xml
HS --> C: device description + Application-URL
@enduml
```

## Scenario 2: Application launch with additional data (from the spec)

Taken from the DIAL spec, Annex C ("DIAL REST Service: Application Launch"), with `phony` as app X.

```plantuml
@startuml
participant "First-Screen App\n(phony)" as F #lightgreen
participant "DIAL REST Service" as S #lightblue
participant "DIAL Client" as C

opt
  note right of S: Does app exist or have data\nto communicate to client?
  C -> S: (1) GET <Application-URL>/phony
  S --> C: (2) 200 OK
end
note right of C: Launch app phony
C -> S: (3) POST <Application-URL>/phony (optional arguments)
S -> F: (4) Launch app (additionalDataUrl)
S --> C: (5) 201 CREATED w/ LOCATION header
opt additionalData flow
  F -> S: (6) POST /apps/phony/dial_data (additionalData)
  C -> S: (7) GET <Application-URL>/phony
  S --> C: (8) 200 OK (XML: additionalData)
end
C -> F: (9) DIAL client(s) communicate directly with first-screen app
@enduml
```

## Scenario 3: The same launch through our building blocks

```plantuml
@startuml
participant "DIAL Client" as C
participant "HttpSession\n[session strand]" as HS #lightblue
participant "Routes" as R #lightblue
participant "AppDispatcher\n[app strand]" as D #lightgreen
participant "AppController\n(Default or subclass)" as AC #lightgreen
participant "AppLauncher" as L #lightgreen
participant "phony\n(child process)" as F

C -> HS: POST /apps/phony (payload)
HS -> R: request (matched by Router)
R -> R: CORS check
R -> D: dispatch(launch, AppRequest)
D -> AC: launch(request) on app strand
alt stopped
  AC -> L: launch(LaunchRequest, exitCallback)
  L -> F **: start
  AC --> D: 201 Created, LOCATION .../apps/phony/run
else already running
  AC --> D: 201 Created (ADR-7)
else launcher error
  AC --> D: Err (e.g. 503)
end
D --> HS: Result<Reply> (posted back to session strand)
HS --> C: HTTP response

... later ...
F -> L: exits (on its own or after DELETE)
L -> D: exitCallback(code) (any thread)
D -> AC: onExit("phony", code) on app strand -> state stopped
@enduml
```
