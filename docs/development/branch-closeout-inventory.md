# Branch Closeout Inventory

Inventory date: 2026-07-23

The inventory started from:

```text
develop: 7fb0dc680c2ff90418d836138c503949d8fbe25d
master:  518b100169ce41d318f3a971c9b149c47aa89a5c
upstream: b5581cb3717e1152e7fcd1fb387a712d985e8850
local branches: 9
remote branches: 9, excluding origin/HEAD
```

`git fetch --all --prune` completed before classification. All four attached
organization worktrees were clean. The closeout branch
`docs/branch-closeout` was created afterward and is not part of the starting
count.

Ahead and behind values below use `ahead/behind` relative to the named branch.

## Current Branches

| Branch | Location | SHA | Upstream | vs develop | vs master | Merge base with develop | Git merged into develop | Patch in develop | Latest commit | Purpose | Disposition |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `master` | local and `origin` | `518b100169ce41d318f3a971c9b149c47aa89a5c` | `origin/master` | 1/5 | 0/0 | `abc985f1528814219562613f24e3e464533e7abf` | No | Release content is represented; release merge commit remains unique | 2026-07-16 | Stable v0.4.0 release history | KEEP ACTIVE |
| `develop` | local and `origin` | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | `origin/develop` | 0/0 | 5/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | Yes | Yes | 2026-07-19 | Integration branch | KEEP ACTIVE |
| `upstream/master` | remote | `b5581cb3717e1152e7fcd1fb387a712d985e8850` | None | 0/62 | 0/58 | `b5581cb3717e1152e7fcd1fb387a712d985e8850` | Yes | Yes | 2024-07-01 | Sigmdel upstream tracking reference | KEEP ACTIVE |
| `style/static-analysis-cleanup` | local and `origin` | `3c387bda4c308f330c7fb825af7de9baf63af69e` | `origin/style/static-analysis-cleanup` | 7/0 | 12/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | No | No; seven unique commits | 2026-07-21 | Formatting policy, mechanical C formatting, shell safety, lint gates, beta guidance | MERGE INTO DEVELOP |
| `refactor/repository-layout-source` | local, `origin`, worktree | `dbb9f15dd8a7c80b48f58bab1aff5d3e2573b1cb` | `origin/refactor/repository-layout-source` | 11/0 | 16/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | No | No; includes the seven style commits and four source-layout commits | 2026-07-23 | Move maintained C sources under `src/`, update build paths, record move manifest | REBASE AND REVIEW |
| `refactor/repository-layout-support` | local, `origin`, worktree | `41f0c207e222524fe23eff8726d338966de5fbd7` | `origin/refactor/repository-layout-support` | 12/0 | 17/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | No | No; includes style and the first two source-layout commits | 2026-07-22 | Organize documentation, packaging, scripts, tests, and support files | REBASE AND REVIEW |
| `cleanup/remove-obsolete-files` | local, `origin`, worktree | `f8735186f9a51b59d8da154a1a1c5cfad8fd2dc8` | `origin/cleanup/remove-obsolete-files` | 14/0 | 19/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | No | No; includes support layout and two cleanup commits | 2026-07-22 | Archive superseded support files under reviewed research paths | REBASE AND REVIEW |
| `docs/repository-architecture` | local, `origin`, worktree | `1638fdc00188c5f3da5dd1a8eda3d1883167b574` | `origin/docs/repository-architecture` | 16/0 | 21/1 | `7fb0dc680c2ff90418d836138c503949d8fbe25d` | No | No; includes cleanup and two final documentation commits | 2026-07-22 | Repository inventory, layout rationale, architecture navigation | REBASE AND REVIEW |
| `test-simplification/mochad-redux` | local and `origin` | `ce4aead16354aa9adeb4fd61bfcf77a39fc909ef` | `origin/test-simplification/mochad-redux` | 4/3 | 6/1 | `2af2abfb96a4905c9a03a91d9a9fcf6d08a2a7e6` | No | No; useful files are mixed with stale paths and workflow assumptions | 2026-07-17 | Fast/release CI split, test strategy, release evidence, hardware-lab policy and scripts | CHERRY-PICK SELECTED COMMITS |
| `backup/native-install-contract-before-rework` | local | `8a31af231ba31ecdf4b459b4c61d57422b8dd264` | None | 1/23 | 1/19 | `9fc2bdec23ec5e380752d53bbe7d331793ef3bcd` | No | No exact patch; behavior is replaced by `7b7a1c6` and `7fb0dc6` | 2026-07-09 | Pre-rework native install safety backup | SUPERSEDED |

## Working Status

The canonical checkout and these attached worktrees reported zero changes:

```text
docs/repository-architecture
cleanup/remove-obsolete-files
refactor/repository-layout-source
refactor/repository-layout-support
```

Branches without a worktree had no separate uncommitted state to preserve.

## Historical Safety Branches

The historical PR heads were fetched into temporary review refs without
restoring deleted remote branches.

| Historical branch | PR | Head SHA | Evidence | Disposition |
| --- | --- | --- | --- | --- |
| `fix/nonblocking-client-output` | #2 merged; duplicate #1 merged | `424db450fe87474a7c9a2fb5cb777af4a75c3420` | Head is an ancestor of `origin/develop`; ahead/behind is 0/31 | ALREADY INCORPORATED |
| `fix/strict-command-parser` | #3 merged | `88ef09090ea80cee5b6c628600495ff8d9b85cc3` | Head is an ancestor of `origin/develop`; ahead/behind is 0/30 | ALREADY INCORPORATED |
| `fix/usb-tx-lifecycle` | #4 closed unmerged against `master` | `c872c2f04e480876ae70e90543968520652095b7` | Despite PR state, head is an ancestor of `origin/develop`; ahead/behind is 0/30 | ALREADY INCORPORATED |

The USB lifecycle branch must not be reopened unchanged. Its complete head
already entered `develop` through later integration history.

## Organization Dependency

The branch ancestry is:

```text
develop
  -> style/static-analysis-cleanup
       -> refactor/repository-layout-source
       -> refactor/repository-layout-support
            -> cleanup/remove-obsolete-files
                 -> docs/repository-architecture
```

The source and support layout tips diverged after their first two shared
source-layout commits. The source branch contains two later source-build and
manifest corrections that the support branch does not contain. Therefore the
support branch must be reconstructed on the final source-layout result rather
than merged in its current form.

## Test-Simplification Findings

The branch contains worthwhile test strategy, release-evidence, and
hardware-lab material, but it predates the completed native-install work and
current workspace policy. Merging it directly would delete current native
setup files and restore stale paths.

Its hardware script also transmits `rf A1 on`; current workspace policy permits
development RF only on house code `D`. No transmitting script from this branch
may be merged unchanged. Selected material must be reconstructed after the
layout branches, updated to current paths, and validated independently.

## Validation Requirements

Every merge candidate requires:

- ClangFormat
- ShellCheck
- `git diff --check`
- strict libusb-free compile
- unit tests
- TCP diagnostics and ports 1099, 1100, and 1101 compatibility
- full libusb build when prerequisites are available
- staged `DESTDIR` install
- native setup validation
- version consistency
- source archive validation
- repository hygiene

Formatting and repository-layout changes are `NOT APPLICABLE` for hardware
validation because they do not alter USB discovery, transfers, transmission,
receive decoding, controller startup/shutdown, udev permissions, or locking.

## Evidence

Inventory commands included:

```text
git branch --all --verbose --no-abbrev
git branch --merged origin/develop
git branch --no-merged origin/develop
git rev-list --left-right --count
git merge-base
git merge-base --is-ancestor
git cherry -v origin/develop
git diff --stat origin/develop...<branch>
git worktree list --porcelain
```

Validated and merged style branch:

```text
branch: style/static-analysis-cleanup
SHA: 3c387bda4c308f330c7fb825af7de9baf63af69e
PR: #6, merged into develop on 2026-07-23
restricted Linux evidence:
  /srv/x10-dev/evidence/20260723T191717Z-mochad-redux-3c387bda4c30
result: 12 PASS, 1 NOT APPLICABLE, 0 FAIL
```

The local disposable run is retained at:

```text
/Users/monsterray/Projects/Portainer Server Setup/X10/evidence/
20260723T191546Z-mochad-redux-3c387bda4c30
```

Its TCP check is `FAIL` because the macOS application sandbox denied listener
creation. The same exact bundle passed TCP diagnostics on the restricted Linux
validator, so the local result is retained as environment evidence rather than
treated as a source defect.
