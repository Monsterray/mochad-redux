# Security Policy

`mochad-redux` is a local automation daemon that exposes TCP listeners and talks
to USB X10 controllers. Security reports should focus on issues that affect
local network exposure, unsafe parsing, privilege boundaries, Docker
deployment, or host USB access.

## Supported Versions

Until the first stable release, security fixes target the active `develop`
branch and the latest tagged release when practical.

## Reporting a Vulnerability

Until a private security contact is published, open a GitHub issue only for
non-sensitive security hardening requests. For issues that could expose systems
or users, contact the maintainer privately before publishing details.

Do not include:

- Private hostnames or IP addresses.
- Credentials.
- Full home-network topology.
- Unnecessary USB serial details.
- Logs unrelated to the vulnerability.

## Expectations

This daemon should generally be deployed on trusted local networks. It does not
provide authentication for the historical `mochad` TCP protocol. Do not expose
the TCP listener directly to the public internet.
