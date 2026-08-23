# How the daemon is supervised on Windows

Type: grilling
Status: open
Blocked by: none

## Question

`daemon()` has 2 call sites and no Windows equivalent. Decide the model: a real
Windows service (needs a service main, a control handler, and registration),
foreground-only with supervision left to the user or a wrapper, or both with the
service path optional.

This also decides where logging goes, so ticket 06 depends on it: a service has
no stderr to write to, a foreground process does. The existing
`openlog(DAEMON_NAME, LOG_PID | (foreground ? LOG_PERROR : 0), LOG_LOCAL5)`
already encodes exactly that split on POSIX.
