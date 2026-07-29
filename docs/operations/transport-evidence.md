# Transport Evidence

The main TCP listener provides a read-only `evidence` diagnostic command. It
returns one newline-delimited JSON object containing the most recent
Redux-owned transport facts:

```sh
printf 'evidence\n' | nc localhost 1099
```

The recorder is an in-memory ring limited to 32 facts. `dropped` reports facts
overwritten by newer observations. Evidence resets when the daemon restarts
and is not a backup or audit log.

## Scope

The response may contain:

- Redux-local command and attempt IDs;
- command acceptance or rejection;
- USB queue, submission, completion, failure, cancellation, and timeout facts;
- genuine controller acknowledgement, only when byte `0x55` was received;
- one terminal result per attempt;
- normalized receive direction, transport, duplicate count, and decode result.

Legacy TCP cannot carry a bridge command ID, so
`external_correlation` is always `unavailable`. Consumers must not correlate
events by timing. Unknown fields must be tolerated so additive diagnostics do
not break older clients.

## Interpretation

`redux.usb_completed` means libusb completed the local OUT transfer. It does
not prove controller acknowledgement or physical activation.
`redux.controller_acked` means Redux received byte `0x55`; it is not physical
confirmation. CM19A attempts normally terminate after USB completion because
the controller does not provide the CM15A powerline acknowledgement behavior.
An ACK timeout produces an `unknown` terminal outcome and does not trigger an
automatic retransmission. Shutdown records unsubmitted queued attempts as
`cancelled`; an active attempt whose libusb callback cannot be observed before
the bounded shutdown deadline is `unknown`.

Redux does not emit device state, state confidence, bridge correlation, MQTT
facts, Home Assistant facts, or physical confirmation. Human or external
sensor observations belong in approved validation evidence.

The runtime does not currently embed an exact source SHA, so `emitter.sha` is
`null`. Support and release evidence must bind the diagnostic output to the
tested binary and exact repository SHA externally.
