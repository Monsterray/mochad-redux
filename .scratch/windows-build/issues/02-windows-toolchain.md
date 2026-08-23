# Choose the Windows toolchain and build system

Type: grilling
Status: open
Blocked by: none

## Question

Nothing can be compiled until this is settled, so it gates most of the map.

The options carry very different costs. MSVC needs a non-Autotools build (CMake
or a project file) and is the least POSIX-tolerant. MinGW-w64 under MSYS2 can
often run Autotools largely unchanged and tolerates POSIX idioms, at the cost of
a toolchain the project does not currently use. Cross-compiling from Linux with
`mingw-w64` is attractive because CI already runs on Linux and the repo already
cross-compiles for ARM.

Decide the toolchain, whether Autotools stays the single build system or gains a
parallel one, and whether the build is native or cross-compiled.

Constraint: there is no Windows C toolchain on the development machine, so a
cross-compile from Linux is the only option verifiable here.
