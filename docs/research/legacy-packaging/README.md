# Legacy Packaging

This directory preserves inherited systemd and udev files that have been
superseded by configurable templates under `packaging/linux/`.

- `systemd/mochad.service` hardcodes the binary path and service identities.
- The static udev rules hardcode group `x10`; one also couples device discovery
  directly to systemd activation.

Current installation uses `mochad-redux-setup` to render the maintained
templates. Nothing under this research directory is installed or activated.
