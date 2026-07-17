# Experimental Mochad JSON API Design

This document describes a future generic JSON API for `mochad-redux`.

The API is intentionally daemon-oriented and integration-neutral. It must not
encode Home Assistant, MQTT, entity registry, discovery, or platform concepts
into the daemon wire protocol. Home Assistant, MQTT bridges, CLI tools, and
other clients should all consume the same generic X10-oriented API.

## Status

Status: design only.

The JSON API is not part of the current runtime. Implementation should wait
until the `v0.4.x` runtime-hardening work has enough hardware validation and
release evidence.

When implemented, the JSON listener should be:

- Optional.
- Disabled by default while experimental.
- Separate from the existing compatibility listeners.
- Bound through the same validated listener configuration path as the other
  TCP services.
- Clearly logged at startup as enabled, disabled, or listening.

Default proposed listener:

```text
port: 1102
framing: UTF-8 JSON object per newline
protocol: JSON-RPC 2.0
```

Future configuration surface:

```text
MOCHAD_JSON_ENABLED=false
MOCHAD_JSON_PORT=1102

--enable-json
--disable-json
--json-port 1102
```

These names are reserved by this design, not implemented in the current
runtime.

## Goals

- Provide a generic machine-readable API for mochad state, diagnostics, and
  events.
- Preserve existing legacy TCP behavior on port `1099`.
- Preserve the Flash XMLSocket-compatible listener on port `1100`.
- Preserve the OpenRemote-compatible listener on port `1101`.
- Give modern bridges a stable local push API without scraping legacy text.
- Keep command and event semantics X10-oriented rather than integration-
  oriented.
- Make every message deterministic enough for regression tests.
- Protect the daemon event loop from slow or stuck clients.

## Non-Goals

- Do not replace the existing text protocol.
- Do not add Home Assistant entity concepts to the daemon.
- Do not publish MQTT topics from `mochad-redux`.
- Do not expose Home Assistant discovery payloads.
- Do not require clients to use JSON for existing legacy functionality.
- Do not enable the JSON listener by default while experimental.

## Relationship To Existing Listeners

The existing listeners remain compatibility interfaces:

```text
1099 main TCP listener
  newline-delimited legacy commands and events

1100 Flash XMLSocket-compatible listener
  legacy client compatibility
  NUL-delimited event framing
  not a structured XML API

1101 OpenRemote-compatible listener
  legacy OpenRemote behavior

1102 JSON listener
  optional experimental JSON-RPC 2.0 API
  UTF-8 JSON object per newline
```

Bridge-oriented diagnostic commands such as `hello`, `capabilities`, `health`,
`config`, and `clients` remain newline-delimited on the main listener. The JSON
listener should expose equivalent information through JSON-RPC methods instead
of changing the main listener contract.

## Internal Event Model

Before adding the JSON listener, introduce one normalized internal event model.
All outward formats should be generated from this model:

- Legacy text formatter.
- XMLSocket formatter.
- OpenRemote formatter.
- JSON notification formatter.

Proposed internal structure:

```c
typedef enum {
    MOCHAD_EVENT_RF,
    MOCHAD_EVENT_PL,
    MOCHAD_EVENT_RFSEC,
    MOCHAD_EVENT_RFCAM,
    MOCHAD_EVENT_STATUS,
    MOCHAD_EVENT_DIAGNOSTIC,
    MOCHAD_EVENT_UNKNOWN
} mochad_event_kind_t;

typedef enum {
    MOCHAD_DIRECTION_RX,
    MOCHAD_DIRECTION_TX,
    MOCHAD_DIRECTION_INTERNAL
} mochad_direction_t;

typedef struct {
    uint64_t sequence;
    struct timeval timestamp;
    mochad_event_kind_t kind;
    mochad_direction_t direction;
    char house;
    int unit;
    char address[4];
    char addresses[16][4];
    size_t address_count;
    char function[32];
    char transport[16];
    char raw_text[256];
} mochad_event_t;
```

The exact fields can evolve during implementation, but the invariant is more
important than the initial shape: parsing and controller code should produce an
internal event once, then formatters should render that event for each listener.

Product-model behavior belongs in clients, not in `mochad-redux`. The normalized
event model should preserve protocol facts such as multiple accumulated
addresses before one function, house/function events, direction, transport, and
raw text. It should also keep standard RF identities separate from RF security
identities so bridges can distinguish normal HouseUnit activity from security
sensor events without relying on product names.

## Sequencing

Each runtime event should receive a monotonically increasing unsigned sequence
number.

```text
sequence starts at 1 for each daemon start
sequence increments once per internal event
sequence never depends on client count or output format
```

Sequence numbers help bridges detect missed events after reconnect. They are
not a durable event log; if a client disconnects, it must request a fresh
snapshot after reconnecting.

## Identity

The API should expose two daemon identity values:

```text
instance_id
  persistent daemon installation identity
  generated once and stored in the configured state directory
  stable across daemon restarts

session_id
  generated at process startup
  changes on every daemon start
```

If no writable state directory is configured, the daemon may run with an
ephemeral `instance_id`, but it must report that fact in handshake and health
responses.

## Framing

Every JSON API message is one UTF-8 JSON object followed by `\n`.

Rules:

- No embedded raw newline outside JSON strings.
- Clients may pipeline requests.
- Server responses must include the matching request `id`.
- Server notifications must omit `id`.
- Invalid JSON receives a JSON-RPC parse error when possible.
- Oversized input lines should be rejected without growing unbounded memory.

## JSON-RPC 2.0

Requests:

```json
{"jsonrpc":"2.0","id":1,"method":"mochad.handshake","params":{"client":"example","protocol_version":1}}
```

Responses:

```json
{"jsonrpc":"2.0","id":1,"result":{"protocol_version":1}}
```

Errors:

```json
{"jsonrpc":"2.0","id":1,"error":{"code":-32601,"message":"Method not found"}}
```

Notifications:

```json
{"jsonrpc":"2.0","method":"mochad.event","params":{"sequence":42}}
```

Use the standard JSON-RPC error codes:

```text
-32700 parse error
-32600 invalid request
-32601 method not found
-32602 invalid params
-32603 internal error
```

Implementation-specific errors should use the reserved server range:

```text
-32000 daemon error
-32001 unsupported protocol version
-32002 subscription required
-32003 output queue full
-32004 command rejected
-32005 hardware unavailable
```

## Protocol Negotiation

The first client request should be `mochad.handshake`.

The server may allow read-only methods before handshake for debugging, but a
well-behaved client should negotiate first.

Request:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "mochad.handshake",
  "params": {
    "client": "mqtt-mochad-bridge",
    "client_version": "0.1.0",
    "protocol_version": 1
  }
}
```

Response:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "protocol_version": 1,
    "server": "mochad-redux",
    "server_version": "0.4.x",
    "upstream_base": "0.1.18",
    "instance_id": "b57d9a7a-1f35-4210-9885-cd8c4a50d51b",
    "session_id": "bc74fef8-c02a-4ce3-8c83-d328ddbf7b3a",
    "capabilities": [
      "health",
      "config.read",
      "state.snapshot",
      "events.subscribe"
    ]
  }
}
```

## Initial Read-Only Methods

Implement read-only methods before command methods.

### `mochad.handshake`

Negotiates protocol version and returns identity information.

### `mochad.health.get`

Returns daemon, USB, controller, listener, queue, and uptime status.

The response should be concise and operationally useful:

```json
{
  "ok": true,
  "uptime_seconds": 123,
  "usb_connected": true,
  "controller": "CM19A",
  "endpoints_ready": true,
  "transfers_ready": true,
  "listeners": {
    "main": {"enabled": true, "port": 1099},
    "json": {"enabled": true, "port": 1102}
  }
}
```

### `mochad.capabilities.get`

Returns API and daemon capabilities, not Home Assistant entity capabilities.

Examples:

```text
legacy.main
legacy.xmlsocket
legacy.openremote
jsonrpc.v1
events.subscribe
state.snapshot
x10.command.send
raw_data
cm19a
cm15a
```

### `mochad.config.get`

Returns sanitized read-only runtime configuration. It must not return secrets.

### `mochad.state.snapshot`

Returns the daemon's current best-known X10 state.

The snapshot must clearly distinguish:

- Known state.
- Unknown state.
- Last observed event sequence.
- Last observed timestamp when available.

### `mochad.events.subscribe_snapshot`

Atomically subscribes the client to event notifications and returns a state
snapshot from the same sequencing point.

This prevents the classic race where a client asks for a snapshot, then
subscribes, and misses an event between the two operations.

Response shape:

```json
{
  "subscribed": true,
  "snapshot_sequence": 42,
  "state": {
    "devices": []
  }
}
```

After this response, the server sends `mochad.event` notifications for events
with sequence numbers greater than `snapshot_sequence`.

## Future Command Method

Only after the read-only API is stable, add:

```text
x10.command.send
```

The method should accept X10 command intent, not legacy mochad text syntax:

```json
{
  "jsonrpc": "2.0",
  "id": 20,
  "method": "x10.command.send",
  "params": {
    "transport": "rf",
    "address": "A1",
    "command": "on"
  }
}
```

The response should confirm acceptance by the daemon, not imply that an X10
device physically changed state:

```json
{"jsonrpc":"2.0","id":20,"result":{"accepted":true}}
```

## Event Notifications

Events use server notifications:

```json
{
  "jsonrpc": "2.0",
  "method": "mochad.event",
  "params": {
    "sequence": 43,
    "session_id": "bc74fef8-c02a-4ce3-8c83-d328ddbf7b3a",
    "timestamp": "2026-07-06T22:15:30.123-07:00",
    "kind": "x10.device",
    "direction": "rx",
    "transport": "rf",
    "address": "A1",
    "command": "on",
    "raw_text": "07/06 22:15:30 Rx RF HouseUnit: A1 Func: On"
  }
}
```

The `raw_text` field is useful for compatibility diagnostics, but clients
should rely on normalized fields when available.

## Slow Client Protection

The JSON listener must not let a slow client block USB processing or the daemon
event loop.

Each JSON client should have:

- A bounded output queue.
- A maximum queued byte count.
- A maximum queued message count.
- Clear disconnect behavior when limits are exceeded.

Suggested first policy:

```text
max queued messages: 256
max queued bytes: 256 KiB
on overflow: send error notification if possible, then disconnect
```

The daemon should log a concise message:

```text
[CLIENT] client id=12 disconnected type=json reason=output_queue_full
```

Do not block the daemon trying to flush a slow JSON client.

## Implementation Order

Keep each step small and reviewable:

1. Add `mochad_event_t` and formatter tests for legacy text output.
2. Route legacy main-listener text output through the event formatter.
3. Route XMLSocket and OpenRemote output through the event formatter.
4. Add internal event sequence generation.
5. Add persistent `instance_id` and per-start `session_id`.
6. Add JSON-RPC parser and serializer tests without opening a socket.
7. Add optional JSON listener configuration, disabled by default.
8. Add bounded per-client output queues.
9. Add `mochad.handshake`.
10. Add `mochad.health.get`.
11. Add `mochad.capabilities.get`.
12. Add `mochad.config.get`.
13. Add `mochad.state.snapshot`.
14. Add `mochad.events.subscribe_snapshot`.
15. Add event notifications.
16. Add bridge/library integration experiments.
17. Add `x10.command.send` only after read-only behavior is stable.

## MQTT Bridge Migration Plan

`mochad-mqtt-bridge` should remain compatible with legacy mochad deployments.

Future bridge configuration:

```text
MOCHAD_PROTOCOL=auto|json|legacy
MOCHAD_JSON_PORT=1102
```

Expected behavior:

```text
auto
  try JSON handshake on 1102
  fall back to legacy 1099 only if JSON is unavailable

json
  require JSON API
  fail clearly if unavailable

legacy
  use current newline-delimited main listener behavior
```

During migration, the bridge should keep its MQTT topics, Home Assistant
discovery, state manager, and command model stable. The daemon protocol should
not learn MQTT topic names or Home Assistant entity concepts.

## aiomochad Library Plan

Create a separate async Python package, proposed name `aiomochad`.

Responsibilities:

- Connect to the JSON API.
- Perform handshake and protocol negotiation.
- Provide typed async methods for health, capabilities, config, snapshot, and
  subscribe-plus-snapshot.
- Expose sequenced events as an async iterator.
- Handle reconnect and resubscribe behavior.
- Preserve raw JSON access for diagnostics.

Non-responsibilities:

- MQTT publishing.
- Home Assistant discovery.
- Entity modeling.
- Docker orchestration.

This library can then serve both `mochad-mqtt-bridge` and Home Assistant.

## Home Assistant Integration Plan

Modernize the existing official Home Assistant `mochad` integration after the
daemon JSON API and `aiomochad` library are stable.

Goals:

- Move from legacy YAML/local polling to config entries.
- Use local push from the JSON API instead of scraping legacy text.
- Keep Home Assistant entity concepts inside the Home Assistant integration.
- Use `aiomochad` for daemon communication.
- Use Home Assistant diagnostics and repairs where appropriate.

The daemon should remain generic. It should expose X10 state, commands,
capabilities, and events; Home Assistant decides how those become entities.

## Validation Requirements

Before enabling this API by default, collect evidence for:

- Legacy main listener compatibility.
- XMLSocket compatibility.
- OpenRemote compatibility.
- JSON parser error handling.
- JSON handshake negotiation.
- Snapshot and subscribe-plus-snapshot race protection.
- Event sequence monotonicity.
- Slow-client disconnect behavior.
- Reconnect behavior in `mochad-mqtt-bridge`.
- CM19A hardware event delivery.
- CM15A hardware event delivery when hardware is available.

The JSON listener should remain disabled by default until this evidence exists.
