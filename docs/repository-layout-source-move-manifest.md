# Source Layout Move Manifest

Reviewed audit base: `3c387bda4c308f330c7fb825af7de9baf63af69e`

Branch: `refactor/repository-layout-source`

This manifest records the maintained C and header relocations approved by the
repository organization audit. Each entry was moved with `git mv`; filenames,
file contents, C symbols, protocol behavior, USB behavior, and installation
behavior were not changed by the move commit.

## Core

| Old path | New path |
| --- | --- |
| `mochad.c` | `src/core/mochad.c` |
| `global.c` | `src/core/global.c` |
| `global.h` | `src/core/global.h` |

## Configuration

| Old path | New path |
| --- | --- |
| `config.c` | `src/config/config.c` |
| `config.h` | `src/config/config.h` |
| `version.h` | `src/config/version.h` |

`VERSION` remains the root source of truth. The release scripts continue to
generate and validate `src/config/version.h`.

## Network

| Old path | New path |
| --- | --- |
| `diagnostics.c` | `src/net/diagnostics.c` |
| `diagnostics.h` | `src/net/diagnostics.h` |
| `socket_io.c` | `src/net/socket_io.c` |
| `socket_io.h` | `src/net/socket_io.h` |

## USB

| Old path | New path |
| --- | --- |
| `usb_endpoint_selection.c` | `src/usb/usb_endpoint_selection.c` |
| `usb_endpoint_selection.h` | `src/usb/usb_endpoint_selection.h` |
| `x10_write.c` | `src/usb/x10_write.c` |
| `x10_write.h` | `src/usb/x10_write.h` |

## X10

| Old path | New path |
| --- | --- |
| `decode.c` | `src/x10/decode.c` |
| `decode.h` | `src/x10/decode.h` |
| `encode.c` | `src/x10/encode.c` |
| `encode.h` | `src/x10/encode.h` |
| `mochad_event.c` | `src/x10/mochad_event.c` |
| `mochad_event.h` | `src/x10/mochad_event.h` |
| `x10state.c` | `src/x10/x10state.c` |
| `x10state.h` | `src/x10/x10state.h` |

## Mechanical Build Updates

- The root `Makefile.am` remains non-recursive and lists the new paths.
- `AM_CPPFLAGS` exposes the five source-category include directories so quoted
  include names remain unchanged.
- Automake uses `subdir-objects`.
- `AC_CONFIG_SRCDIR` points to `src/core/mochad.c`.
- Test, formatting, strict-compile, libusb-stub, version, and release scripts
  use the relocated paths.
- Source archives include this manifest and every relocated source file.
- Generated Autotools files and local build products remain untracked.

## Exceptions

None. All 22 approved maintained C and header files were moved mechanically.
No source file required a content edit in the move commit.
