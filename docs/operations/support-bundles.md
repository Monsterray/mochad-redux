# Sanitized Support Bundles

The Redux support collector creates a local, shareable archive from reviewed
facts supplied on standard input. It never connects to mochad, opens USB
hardware, reads the host environment, walks directories, uses Docker, uploads
data, or accepts arbitrary attachments.

Support bundles are diagnostics, not backups. Review every archive before
sharing it.

## Input

Prepare one UTF-8 JSON object:

```json
{
  "component": {
    "sha": "0123456789abcdef0123456789abcdef01234567",
    "ref": "v0.5.0",
    "dirty": false
  },
  "version": "0.5.0",
  "diagnostics": {
    "hello": {
      "ok": true,
      "name": "mochad-redux",
      "version": "0.5.0",
      "upstream_base": "mochad 0.1.18"
    },
    "health": {
      "ok": true,
      "usb_connected": true,
      "controller": "CM19A"
    }
  },
  "log": "[STARTUP] mochad-redux 0.5.0\n"
}
```

Allowed diagnostic names are `hello`, `capabilities`, `health`, `config`, and
`version`. Unknown input categories are rejected. Capture these facts
separately, review them, and construct the input without production secrets.
The collector does not contact a daemon on the operator's behalf.

Run:

```sh
python3 scripts/support/collect_support_bundle.py \
  --output ./mochad-redux-support.tar.gz < reviewed-support-input.json
```

The input document is limited to 256 KiB. Logs default to the first 200 lines
and 64 KiB; smaller limits can be selected with `--max-log-lines` and
`--max-log-bytes`.

## Security Contract

The collector:

- accepts no input file or directory paths;
- allowlists Redux-owned diagnostic fields;
- pseudonymizes network, path, and standard X10 device identities;
- removes security RF identifiers and recognized secret values;
- uses mode `0700` for staging and `0600` for staged files and the archive;
- scans staged filenames and contents;
- opens and scans every member of the completed archive;
- refuses to overwrite an existing archive;
- fails closed and publishes no archive when unresolved findings remain.

The archive contains `manifest.json`, `scan-result.json`, and only the supplied
allowlisted facts. Both metadata documents use support-bundle manifest schema
version 1.

Secret scanning is a guardrail, not proof that disclosure is safe. Inspect the
archive with:

```sh
tar -tzf mochad-redux-support.tar.gz
tar -xOf mochad-redux-support.tar.gz manifest.json
tar -xOf mochad-redux-support.tar.gz scan-result.json
```

Do not supply complete environment files, credentials, private keys, broker or
Home Assistant configuration, unrestricted logs, production data, or raw
security-device identifiers.
