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

- `master` should stay close to the public release baseline and upstream sync
  points.
- `develop` is the integration branch for ongoing work.
- Feature branches should branch from `develop` and merge back through review.

Avoid mixing upstream synchronization with feature work in the same commit.
Small, single-purpose commits will make future rebases and reviews easier.

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

## Local build check

To compile the non-USB source files without installing libusb, run:

```sh
sh tools/compile_without_libusb.sh
```

For a warning-as-error pass, run:

```sh
sh tools/compile_without_libusb.sh --strict
```

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
