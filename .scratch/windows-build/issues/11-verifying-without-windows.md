# How a Windows build gets verified

Type: grilling
Status: open
Blocked by: 02

## Question

There is no Windows C toolchain on the development machine, so "it compiles" is
currently unfalsifiable here. Decide what evidence counts and who produces it: a
Linux cross-compile with `mingw-w64` (verifiable here, proves compilation but
not runtime), a GitHub Actions `windows-latest` job, or a real Windows host the
user drives.

Settle this before implementation tickets land, so their claims can be checked
rather than asserted. The project already holds this line elsewhere: the support
matrix and the hardware evidence only ever claim what was recorded.
