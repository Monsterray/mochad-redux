# Beta Status

mochad-redux 0.5.0 is a compatibility-focused beta line. It preserves the
legacy main TCP listener while improving installation, diagnostics, validation,
and maintainability.

Use a tagged beta release or exact full Git SHA. Do not report results from a
moving development branch.

| Area | Status | Notes |
| --- | --- | --- |
| Strict libusb-free compile, formatting, shell safety, unit tests, diagnostics harness | PASS | Recorded source-level validation passed. |
| Main TCP listener and diagnostics | PASS | Native Linux evidence and the exact-SHA macOS CM19A run cover the main listener. |
| XMLSocket and OpenRemote listener smoke tests | NOT RUN | No new macOS runtime evidence was collected for the optional legacy listeners. |
| CM19A native foreground receive/transmit/shutdown | PASS | Recorded on Linux and on macOS 13 x86_64; see the linked hardware evidence. |
| CM19A hotplug event reporting | PASS | A locked macOS test recorded removal, arrival, and truthful unavailable/ready diagnostics. |
| CM19A automatic recovery after replug | PASS | Two locked native macOS cycles reopened the same CM19A, restarted transfers, and restored RF receive/transmit at exact SHA `6dcc3473`. |
| CM19A in-flight shutdown | HARDWARE REQUIRED | Requires a separate approved locked hardware-lab session. |
| CM15A USB receive/transmit/recovery | HARDWARE REQUIRED | Requires CM15A hardware and recorded physical evidence. |
| Module-specific behavior | HARDWARE REQUIRED | Requires a controller, module, and physical result. |

Do not run two mochad processes against one controller. Do not expose the
daemon ports publicly. Before reporting a result, remove credentials, private
hostnames, and sensitive security-device identifiers from logs.
