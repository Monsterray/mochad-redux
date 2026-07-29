# Native Backup and Isolated Restore

`scripts/backup/mochad-redux-backup` preserves the native configuration owned
by mochad-redux. It works offline and never contacts systemd, udev, USB, MQTT,
or a running daemon.

The archive is labelled **unsanitized** because it contains operational
configuration and host metadata. Recognized credential fields are replaced by
external placeholders, but the archive still requires private storage and
human review before sharing.

## Backup Scope

The schema version 1 archive contains:

- `/etc/mochad-redux/mochad.conf`, when present;
- checksums and ownership metadata from
  `/var/lib/mochad-redux/managed-files`;
- the exact mochad-redux version and 40-character Git SHA;
- platform, compatibility, restore-order, mode, and ownership metadata.

Generated systemd units and udev rules are not copied. Users, groups, secrets,
logs, binaries, USB state, and service state are not copied. Systemd and udev
files are reconstructed later by `mochad-redux-setup` from installed templates.
The source managed-state file hash is retained as evidence; restore regenerates
equivalent records using paths rooted in the isolated target.

Create a backup from a clean source checkout:

```sh
sudo ./scripts/backup/mochad-redux-backup create \
  --repository-sha "$(git rev-parse HEAD)" \
  --output /secure/offline/mochad-redux-backup.tar.gz
```

Use `--root` for a test fixture or mounted offline system. Output files are
mode `0600`, and existing archives are preserved unless `--overwrite-output`
is supplied.

## Inspect and Plan

Inspection validates the schema, archive membership, paths, and SHA-256
checksums:

```sh
./scripts/backup/mochad-redux-backup inspect \
  /secure/offline/mochad-redux-backup.tar.gz
```

Restore is a dry run unless `--apply` is present. Host root `/` is always
refused:

```sh
restore_root="$(mktemp -d)"

./scripts/backup/mochad-redux-backup restore \
  /secure/offline/mochad-redux-backup.tar.gz \
  --root "$restore_root"
```

The dry run performs `inspect -> validate -> plan` without creating a staging
directory or changing the isolated root.

## Isolated Apply

Install the matching package into the isolated root first. Git checkouts need
`./autogen.sh`; release archives already contain `configure`.

```sh
make DESTDIR="$restore_root" install

./scripts/backup/mochad-redux-backup restore \
  /secure/offline/mochad-redux-backup.tar.gz \
  --root "$restore_root" \
  --apply \
  --setup-tool ./scripts/mochad-redux-setup
```

Changed existing files require `--overwrite`. Restore stages and verifies
payloads before activation, uses atomic replacement, invokes the setup tool
only with the isolated `--root`, verifies reconstructed checksums, and rolls
back every tracked file if activation or reconstruction fails. A repeated
restore of identical content is idempotent.

If the recorded installation did not manage systemd or udev, the restore
passes the matching `--no-systemd` or `--no-udev` option to the setup tool.
The setup tool does not create accounts or call `systemctl` or `udevadm` for
an alternate root.

## Secrets and Activation

Credential values are never written into the archive. Placeholders such as
`${EXTERNAL_SECRET:password}` identify values that must be restored from a
separate operator-controlled encrypted backup.

Do not activate the restored configuration until:

1. every placeholder has been supplied securely;
2. isolated configuration validation passes;
3. ownership and modes have been reviewed;
4. generated integration-file checksums match;
5. rollback has been tested.

Promotion from the isolated root to a live host is an explicit administrator
operation outside this tool.
