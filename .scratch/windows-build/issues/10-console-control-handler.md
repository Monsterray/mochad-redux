# Signal handling equivalent on Windows

Type: task
Status: open
Blocked by: 02, 04

## Question

`mydaemon()` installs `sigaction` handlers for SIGINT, SIGTERM and SIGQUIT.
Windows has no SIGQUIT and delivers console events through
`SetConsoleCtrlHandler`; a service instead receives control codes through its
service control handler. Provide the equivalent behind `platform.h` for whichever
supervision model 04 selects, preserving the existing orderly-shutdown
semantics.
