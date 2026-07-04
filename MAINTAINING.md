# Maintaining mochad-redux

This fork is intended to stay easy to compare with and rebase onto the
Sigmdel upstream repository. Keep early work focused on repository hygiene,
build reliability, diagnostics, and configuration improvements before making
protocol or behavioral changes.

## Remotes

- `origin`: `git@github.com:Monsterray/mochad-redux.git`
- `upstream`: `https://github.com/sigmdel/mochad.git`

The `upstream` remote should be fetch-only in normal development. A disabled
push URL helps avoid accidental pushes:

```sh
git remote set-url --push upstream DISABLED
```

## Branches

- `master` is the stable release branch.
- `develop` is the integration branch for ongoing work.
- Feature branches should branch from `develop` and merge back through review.
- Future releases should be prepared by opening a pull request from `develop`
  into `master`.
- Release tags should be created from `master` after the release pull request
  has been reviewed and merged.

Avoid mixing upstream synchronization with feature work in the same commit.
Small, single-purpose commits will make future rebases and reviews easier.

## Release Flow

`v0.3.0` is the current baseline release. The active milestone is the `v0.4.x`
runtime-hardening and observability line.

For `v0.4.x`, prioritize clear startup, shutdown, listener, USB, and client
lifecycle diagnostics before adding protocol features. New commands and larger
modernization work should wait until runtime logs are strong enough for users
to diagnose common deployment failures from the daemon output.

For each release:

1. Finish integration work on `develop`.
2. Run CI and the local checks documented below.
3. Fill out release evidence from
   [validation/release-evidence-template.md](validation/release-evidence-template.md).
4. Complete [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md).
5. Open a pull request from `develop` into `master`.
6. Review the release diff for scope, documentation, compatibility, generated
   artifacts, and validation evidence.
7. Merge into `master`.
8. Tag the release from `master`.

Do not tag releases from `develop`.

## Syncing upstream

Use a linear sync flow when possible:

```sh
git fetch upstream --tags
git checkout master
git merge --ff-only upstream/master
git checkout develop
git rebase master
```

If `develop` has fork-specific commits, resolve conflicts during the rebase and
keep the conflict resolutions scoped to the upstream change being replayed.

## Source Tree Hygiene

Generated autotools files and local build outputs should not obscure source
review. Follow [docs/generated-artifacts.md](docs/generated-artifacts.md) when
deciding whether generated files belong in a change.

## Local build check

To compile the non-USB source files without installing libusb, run:

```sh
sh tools/compile_without_libusb.sh
```

For a warning-as-error pass, run:

```sh
sh tools/compile_without_libusb.sh --strict
```

Optional compile/analyzer modes:

```sh
sh tools/compile_without_libusb.sh --strict --asan --ubsan
sh tools/compile_without_libusb.sh --clang-tidy
sh tools/compile_without_libusb.sh --clang-format-check
sh tools/compile_without_libusb.sh --cppcheck
sh tools/compile_without_libusb.sh --cppcheck-style
```

`--asan` and `--ubsan` add sanitizer compile flags. The helper compiles object
files only, so these flags validate compiler compatibility for the non-USB
source files; runtime sanitizer coverage should be added later with executable
tests. `--clang-tidy`, `--clang-format-check`, and `--cppcheck` require those
tools to be installed.

The helper searches `PATH`, `/usr/local/opt/llvm/bin`, and
`/opt/homebrew/opt/llvm/bin` for Homebrew LLVM tools.

The default clang-tidy gate disables the Annex K insecure API recommendation
checker because functions such as `snprintf_s`, `memcpy_s`, and `memset_s` are
optional in C11 and are not portable across the supported Linux targets. Keep
using bounded standard C/POSIX APIs with explicit size checks.

The default cppcheck gate focuses on warnings, performance, and portability.
Style suggestions are opt-in through `--cppcheck-style` so they do not block
safety-focused maintenance work.

This compiles `decode.c`, `encode.c`, `global.c`, `x10state.c`, and
`x10_write.c` as object files in a temporary directory. It intentionally skips
`mochad.c`, which owns the libusb dependency and the daemon socket/USB loop.

Install the build dependencies first. On Debian or Ubuntu:

```sh
sudo apt-get install autoconf automake build-essential libtool libusb-1.0-0-dev pkg-config
```

Then run:

```sh
./autogen.sh
./configure
make
```

The CI workflow performs the same build on Ubuntu.

## CI Targets

The CI workflow covers:

- Ubuntu LTS
- Ubuntu Latest
- Debian Stable
- Raspberry Pi ARM cross compile for the libusb-free source files

Supported platform expectations are documented in
[docs/supported-platforms.md](docs/supported-platforms.md).

## v0.4.x Quality Focus

After logging and diagnostics are in good shape, focus on:

- Sanitizer compile coverage through `--asan` and `--ubsan`.
- Static analysis coverage through cppcheck and clang-tidy.
- Compatibility documentation for Linux, Docker, Raspberry Pi, IPv4 defaults,
  and explicit IPv6 opt-in.
- Repeatable hardware validation notes for CM19A and CM15A.

Do not add new protocol features merely to expose diagnostics until the current
daemon lifecycle logs are clear and consistent.

## Safety Fix Verification

The current safety baseline covers:

- USB write bounds before copying to the libusb interrupt buffer.
- X10 queue write bounds before copying to queue records.
- Bounded TCP command remainders with discard-until-delimiter behavior for
  overlong commands.
- Per-client command parser state for normal, XML, and OR20 sockets.
- Endpoint discovery initialization and failure checks.
- Centralized socket close ownership through `del_client()`.
- Clearer decode and accept debug logs.

After changing any of these areas, run:

```sh
sh tools/compile_without_libusb.sh --strict --asan --ubsan
git diff --check
```

If libusb development headers are available, also run the full build:

```sh
./autogen.sh
./configure
make
```

Keep future safety fixes small and individually reviewable. Do not start broad
USB/TCP/protocol/state separation until the concrete safety issue being fixed is
covered by a focused check or documented manual verification.
