# Hardware Lab Setup

This procedure validates a real CM19A without granting automation access to
production services, Docker, or unrestricted host administration.

## Boundaries

The dedicated account is `codex-x10`. It owns `/srv/x10-dev`, belongs to
`x10dev` and `x10`, and must not belong to `sudo` or `docker`.

Lab runs use:

- `/run/lock/x10-hardware.lock`;
- localhost TCP ports `19000-19999` (`19099` by default);
- X10 housecode `D` only;
- no production MQTT credentials or topics;
- no Home Assistant discovery;
- explicit human approval before RF transmission.

## Server Setup

An administrator performs this once:

```sh
getent group x10 >/dev/null || groupadd --system x10
getent group x10dev >/dev/null || groupadd --system x10dev
useradd --create-home --user-group --home-dir /srv/x10-dev \
  --groups x10dev,x10 --shell /bin/bash codex-x10
install -d -o codex-x10 -g x10dev -m 2770 /srv/x10-dev
install -o root -g x10dev -m 0660 /dev/null /run/lock/x10-hardware.lock
```

Do not add `codex-x10` to `sudo` or `docker`. Configure the CM19A udev rule so
USB device `0bc7:0002` is owned by `root:x10` with mode `0660`.

## SSH Setup

The examples use SSH port `2222`. Replace `SERVER` and the administrator name.

On macOS or Linux:

```sh
ssh-keygen -t ed25519 -f ~/.ssh/codex_x10 -C "codex-x10 hardware lab"
scp -P 2222 ~/.ssh/codex_x10.pub admin@SERVER:/tmp/codex_x10.pub
ssh -p 2222 admin@SERVER
```

On Windows PowerShell:

```powershell
ssh-keygen -t ed25519 -f "$env:USERPROFILE\.ssh\codex_x10" -C "codex-x10 hardware lab"
scp -P 2222 "$env:USERPROFILE\.ssh\codex_x10.pub" admin@SERVER:/tmp/codex_x10.pub
ssh -p 2222 admin@SERVER
```

The administrator installs the key:

```sh
sudo install -d -o codex-x10 -g codex-x10 -m 0700 /srv/x10-dev/.ssh
sudo install -o codex-x10 -g codex-x10 -m 0600 \
  /tmp/codex_x10.pub /srv/x10-dev/.ssh/authorized_keys
sudo rm -f /tmp/codex_x10.pub
```

Workstation SSH configuration:

```sshconfig
Host x10-lab
  HostName SERVER
  Port 2222
  User codex-x10
  IdentityFile ~/.ssh/codex_x10
  IdentitiesOnly yes
```

If the host requires OpenSSH certificates, the SSH CA administrator must sign
the lab public key. Do not temporarily enable password authentication.

Verify:

```sh
ssh x10-lab
id
```

Output must include `x10` and `x10dev`; it must not include `sudo` or `docker`.

## Source Validation

Unpushed branches are transferred as Git bundles and validated in disposable
sandboxes. Canonical checkouts remain clean. This path does not use Docker,
production services, or hardware.

## CM19A Session

Start from a clean reviewed checkout:

```sh
scripts/validate/full-libusb-build.sh
scripts/hardware/lab-preflight.sh
scripts/hardware/run-cm19a-validation.sh
```

The default run checks startup, `hello`, `health`, receive observation, and
idle shutdown. It does not transmit.

For a human-approved transmission session:

```sh
X10_TEST_ADDRESS=D1 scripts/hardware/run-cm19a-validation.sh --transmit
```

The runner displays every proposed command and requires an exact approval
phrase. Automated checks may report `PASS`; physical receive or load behavior
remains `HARDWARE REQUIRED` until a person records the observation.

## Future Self-Hosted Runner

A future runner may use labels `x10-hardware`, `cm19a`, and `sc546a` only for
manually dispatched release validation of trusted commits. Do not enable it
for fork pull requests, unreviewed branches, schedules, or automatic pushes.
