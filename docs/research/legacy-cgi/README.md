# Legacy Perl CGI interface (historical — do not deploy)

A web interface for controlling X10 devices, written around 2010 by the original mochad author. It
was previously kept in `cgi/` at the repository root and distributed in release tarballs.

**It is retained for historical reference only. It is not part of the supported interface, is not
distributed, and must not be deployed.**

## Why it was withdrawn

A security review found multiple unauthenticated, remotely reachable defects. These are recorded here
so the code is not mistaken for something merely dated:

| Issue | Location |
| --- | --- |
| **CSRF on state-changing GET.** The form is `method="post"`, but `ReadParse()` treats GET identically. `GET x10.pl?Function=Disarm&House=B` **disarms the security system** from a bare `<img src>` on any page the resident loads. No token, no Referer check, no method restriction. | `x10.pl:135-168` |
| **Command injection into the daemon socket.** `$x10func` is taken verbatim from a `Unit*` parameter. Line 162 runs `tr/ /_/`, which converts spaces but **not newlines**, then line 172 writes `"pl $unitcode $x10func\n"` to the mochad TCP socket. `?UnitB1=On%0Arfsec+0x11+disarm` smuggles a second command. | `x10.pl:162,172` |
| **Reflected XSS** — a raw attacker-controlled parameter *name* is printed unescaped. | `x10.pl:157` |
| **Reflected XSS** — the `Content-Type` header and raw multipart body are echoed unescaped into error pages. `cgi-lib.pl` contains no escaping helper at all. | `cgi-lib.pl:227,280,411-413` |
| **Unauthenticated sensor disclosure.** A bare `GET x10.pl` renders a live, named table of every security sensor (`Front`, `Garage 1`, `Bedroom 1`…), letting an attacker case the house in real time. | `getsensors.pl:34-51` |
| No taint mode (`-T`), `use strict` commented out, on a script that shells out via backticks. | `x10.pl:1,22,25` |
| Hardcoded personal LAN addresses (`192.168.1.24`, `192.168.1.26`), install path, and real room-to-sensor mappings from the author's own house. | `x10.pl:27,31,35` |

The hardcoded addresses and room names make it clear this was one person's home setup rather than a
general-purpose interface, which is why it was withdrawn rather than repaired.

## If you want a web interface

Use the JSON API (`docs/protocol/json-api.md`) behind your own authenticated front end, or the
Home Assistant integration via [mochad-mqtt-bridge](https://github.com/Monsterray/mochad-mqtt-bridge).
