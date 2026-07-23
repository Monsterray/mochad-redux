# Beta Status

mochad-redux 0.4.0 is a compatibility-focused beta line. It preserves the
legacy main TCP listener while improving installation, diagnostics, validation,
and maintainability.

Use a tagged beta release or exact full Git SHA. Do not report results from a
moving development branch.

| Area | Status | Notes |
| --- | --- | --- |
| Strict libusb-free compile, formatting, shell safety, unit tests, diagnostics harness | PASS | Recorded source-level validation passed. |
| Live TCP listener, XMLSocket, and OpenRemote listener smoke tests | NOT RUN | Require runtime evidence in a suitable environment. |
| CM19A and CM15A USB receive/transmit/recovery | HARDWARE REQUIRED | Requires an approved locked hardware-lab session and human observation. |
| Module-specific behavior | HARDWARE REQUIRED | Requires a controller, module, and physical result. |

Do not run two mochad processes against one controller. Do not expose the
daemon ports publicly. Before reporting a result, remove credentials, private
hostnames, and sensitive security-device identifiers from logs.
