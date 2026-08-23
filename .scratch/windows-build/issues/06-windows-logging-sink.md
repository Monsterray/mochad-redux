# Implement the Windows logging sink

Type: task
Status: open
Blocked by: 02, 04

## Question

Provide `syslog`, `openlog`, `closelog` and `setlogmask` on Windows behind
`platform.h`, plus the 8 `LOG_*` severities, `LOG_PID`, `LOG_PERROR`,
`LOG_LOCAL5` and the `LOG_UPTO` macro. All 121 call sites stay untouched.

Decided: **both sinks, selectable at runtime** — Event Log when running as a
service, stderr when in the foreground, mirroring the existing `LOG_PERROR`
split. Event Log needs an event source registered in the registry, so the
registration step and its privilege requirement are part of this ticket unless
04 sends them to the service-installation work.

`setlogmask` and `LOG_UPTO` must keep working: `config.c` maps
`MOCHAD_LOG_LEVEL` onto syslog severities and validates the range against
`LOG_EMERG`..`LOG_DEBUG`.
