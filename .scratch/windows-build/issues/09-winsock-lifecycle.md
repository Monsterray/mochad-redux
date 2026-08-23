# Winsock initialisation and teardown

Type: task
Status: open
Blocked by: 02, 05

## Question

Winsock requires `WSAStartup` before any socket call and `WSACleanup` at exit;
POSIX needs neither. Decide where they belong so no socket call can precede
initialisation — including on early-exit paths such as `--check-config` and
`--print-config` — and confirm teardown runs on the signal-driven shutdown path.
