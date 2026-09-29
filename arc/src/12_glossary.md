# Glossary

| Term | Meaning |
|---|---|
| DIAL | DIscovery And Launch protocol (Netflix), version 2.2.1 |
| First screen | TV / set-top box running the DIAL server and apps |
| Second screen | Phone / tablet running the DIAL client |
| SSDP | Simple Service Discovery Protocol (UPnP 1.1), UDP multicast `239.255.255.250:1900` |
| M-SEARCH | SSDP discovery request; DIAL uses `ST: urn:dial-multiscreen-org:service:dial:1` |
| USN | Unique Service Name in the M-SEARCH response; clients deduplicate servers by it |
| LOCATION | M-SEARCH response header with the device description URL (`/dd.xml`) |
| Application-URL | Header of the device description response; the DIAL REST Service URL (`http://<ip>:<port>/apps`) |
| Application Resource URL | `<Application-URL>/<name>`, e.g. `/apps/phony` |
| Application Instance URL | URL of the running instance, returned in `LOCATION` of 201, e.g. `/apps/phony/run` |
| DIAL payload | Optional POST body (text/plain, >= 4 KB supported) passed to the app on launch |
| additionalDataUrl | URL the first-screen app POSTs key-value data to (`/apps/<name>/dial_data`), relayed to clients in `<additionalData>` |
| phony | Dummy first-screen app used to exercise the server |
