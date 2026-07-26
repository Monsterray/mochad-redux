# Branch Closeout Results

Closeout completed: 2026-07-26

Integration tip after the final implementation PR:
`2e82df31acfd4af00c27774df0df9eec09c65d45`.

Stable `master` remained unchanged at:
`518b100169ce41d318f3a971c9b149c47aa89a5c`.

## Summary

| Result | Count |
| --- | ---: |
| Starting local branches | 9 |
| Starting origin branches, excluding symbolic refs | 9 |
| Pull requests merged before the final report | 8 |
| Branches reconstructed from selected work | 6 |
| Local branches deleted at final cleanup | 16 |
| Origin branches deleted at final cleanup | 15 |
| Auxiliary worktrees removed | 9 |
| Temporary review refs removed | 3 |
| Archival tags created | 0 |

## Merged Work

| PR | Result | Merge commit |
| --- | --- | --- |
| [#6](https://github.com/Monsterray/mochad-redux/pull/6) | Static-analysis and formatting baseline | `0c0b3f718286baac9a87871957cee39b92c2bede` |
| [#7](https://github.com/Monsterray/mochad-redux/pull/7) | Branch inventory and closeout plan | `0cc5e99d7fead4367f58486355b977b7b9ee8e43` |
| [#8](https://github.com/Monsterray/mochad-redux/pull/8) | Reconstructed maintained-source layout | `0fef7b93693913653c50875f1e89bd2a66736743` |
| [#9](https://github.com/Monsterray/mochad-redux/pull/9) | Reconstructed support-file layout | `6a7a5e7839a06c009957bb14a61d629e30e7f0ba` |
| [#10](https://github.com/Monsterray/mochad-redux/pull/10) | Reconstructed obsolete-file archival | `baa5f5587731d93c584baa34ced802fb8bc9bd60` |
| [#11](https://github.com/Monsterray/mochad-redux/pull/11) | Reconstructed repository architecture documentation | `42121286a7a3a2e0b4d0e7cda6ad3e0b2198536a` |
| [#12](https://github.com/Monsterray/mochad-redux/pull/12) | Selected test, release-evidence, and safe hardware-lab work | `c1ac451efd5d03a5265acbac836f0b64a5ff787b` |
| [#13](https://github.com/Monsterray/mochad-redux/pull/13) | Repository validation capability manifest | `2e82df31acfd4af00c27774df0df9eec09c65d45` |

The reconstructed branches preserved useful changes while excluding stale
paths, outdated native-install assumptions, moving development branches, and
the old unapproved `A`-housecode transmission.

## Historical Findings

These historical fixes were already incorporated into `develop`:

- `fix/nonblocking-client-output`, PR #2 head
  `424db450fe87474a7c9a2fb5cb777af4a75c3420`;
- `fix/strict-command-parser`, PR #3 head
  `88ef09090ea80cee5b6c628600495ff8d9b85cc3`;
- `fix/usb-tx-lifecycle`, closed PR #4 head
  `c872c2f04e480876ae70e90543968520652095b7`.

The native-install backup was superseded by the reviewed pure-install and
host-setup commits already in `develop`.

## Validation Tooling Identity

The final campaign used the workspace validation tooling at:

```text
workspace commit:
  90dd49368fc4b08d75180cb46e6f8ead6ccedb04
validator version:
  1.0.0
validation_tool.py SHA-256:
  247bb89dea6b5003f3815cd5b57c3d9c09af3d7f3931db6141d6a0fa189e4b36
validate-bundle.sh SHA-256:
  3e0aa6cbf71b16af7ce7e004a86da6350b07a5a852066fe5a2a37cbb30cdeebe
capability schema version:
  1
capability schema SHA-256:
  6b743d0fee96597fa6d31fa0e64ebcd2f06a4568c928cb0c24fd4efd5c3b869f
evidence record schema version:
  2
evidence schema SHA-256:
  4cf3c6107d81797b67173b3fa13866be5a20d5e45c9f305baaac2fecc52c1ddf
mochad-redux capability manifest SHA-256:
  3b449167b93e37e1532ad7a96abdf15ee9dc4bd5644003881ceac57924177149
```

## Deleted Branches

Local:

```text
backup/native-install-contract-before-rework
cleanup/remove-obsolete-files
cleanup/remove-obsolete-files-v2
docs/branch-closeout
docs/repository-architecture
docs/repository-architecture-v2
refactor/repository-layout-source
refactor/repository-layout-source-v2
refactor/repository-layout-support
refactor/repository-layout-support-v2
style/static-analysis-cleanup
test-simplification/mochad-redux
test/validation-evidence-v2
codex/validation-capabilities
docs/final-closeout-report
docs/final-closeout-report-v2
```

Origin:

```text
cleanup/remove-obsolete-files
cleanup/remove-obsolete-files-v2
docs/branch-closeout
docs/repository-architecture
docs/repository-architecture-v2
refactor/repository-layout-source
refactor/repository-layout-source-v2
refactor/repository-layout-support
refactor/repository-layout-support-v2
style/static-analysis-cleanup
test-simplification/mochad-redux
test/validation-evidence-v2
codex/validation-capabilities
docs/final-closeout-report
docs/final-closeout-report-v2
```

No archival tag was needed. GitHub PRs and the inventory preserve historical
intent and authorship.

## Validation

Each merge candidate passed five GitHub checks: C format and shell safety,
Ubuntu LTS, Ubuntu Latest, Debian Stable, and Raspberry Pi Cross Compile.

Restricted disposable Linux bundle evidence:

| Candidate | Result | Evidence |
| --- | --- | --- |
| Static analysis | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T191717Z-mochad-redux-3c387bda4c30` |
| Source layout | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T193036Z-mochad-redux-a587849da4d2` |
| Support layout | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T193619Z-mochad-redux-5f04724042dd` |
| Obsolete-file archival | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T194051Z-mochad-redux-ce222c149cdd` |
| Architecture documentation | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T194718Z-mochad-redux-1dd06a4167e2` |
| Validation evidence and lab policy | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/20260723T200154Z-mochad-redux-81a82c337a3e` |
| Capability manifest | 12 PASS, 0 FAIL, hardware NOT APPLICABLE | `/srv/x10-dev/evidence/cache/b5ed7e664afe30dc2e55a052b22696eaf674e08c382cd231766f34f9a796994f` |

The source suites covered ClangFormat, ShellCheck, strict libusb-free compile,
unit tests, TCP diagnostics, full libusb build, staged install, native setup,
version consistency, source archives, and repository hygiene.

## Explicit Limits

- Isolated `mochad-docker` pinned-SHA runtime validation: **NOT RUN**. It
  requires the separate non-production Docker integration runner.
- CM19A/CM15A physical validation: **NOT APPLICABLE** to the closeout changes.
  No closeout branch changed USB discovery, transfers, transmission, receive
  decoding, controller startup or shutdown, udev permissions, or hardware
  locking.

## Final Branch State

Maintained local branches:

```text
develop
master
```

Maintained origin branches:

```text
origin/develop
origin/master
```

The read-only `upstream/master` tracking ref remains for future upstream
archaeology and compatibility comparisons.
