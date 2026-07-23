# Branch Closeout Plan

Plan date: 2026-07-23

Normal changes target `develop`. `master` remains the stable release branch.
No branch in this closeout is permitted to bypass pull-request review.

## Merge Order

| Order | Branch | Purpose | Dependency | Risk | Validation | Review | Post-merge action |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `style/static-analysis-cleanup` | Establish formatting/lint baseline and reviewed shell fixes | Starting `develop` | Medium due mechanical diff | Restricted Linux bundle: 12 PASS, hardware NOT APPLICABLE; five GitHub checks PASS | PR #6 reviewed and merged | Rebuild source layout on updated `develop` |
| 2 | `docs/branch-closeout` | Record inventory, evidence, dependency order, and results | PR #6 | Low | Markdown, links, `git diff --check`, hygiene | Confirm classifications and deletion criteria | Update results after each merge |
| 3 | reconstructed source-layout branch | Move maintained C/header files under `src/` and update build references | Style PR | Medium, mechanical paths | Complete source suite and pinned-SHA packaging check | Compare move manifest and exact path rewrites | Retire old source-layout branch after merge |
| 4 | reconstructed support-layout branch | Organize packaging, docs, scripts, and tests by ownership | Source layout PR | Medium, path-heavy | Complete source suite, staged install, source archive | Verify install/archive/runtime paths | Retire old support-layout branch after merge |
| 5 | reconstructed obsolete-file cleanup | Move approved historical files to documented research locations | Support layout PR | Low to medium | Full references, archive, staged install, hygiene | Confirm every removed live path has a replacement | Retire old cleanup branch after merge |
| 6 | reconstructed architecture documentation | Publish final inventory, layout rationale, and navigation | Cleanup PR | Low | Links, source archive, hygiene | Documentation accuracy | Retire old architecture branch after merge |
| 7 | reconstructed test/hardware evidence work | Preserve selected test strategy, CI, and lab evidence without stale paths or house-code A transmission | Final layout | Medium | Fast/release workflow review, source suite, ShellCheck; no transmission | Confirm release gates remain explicit and hardware uses D only | Retire test-simplification branch after merge |

## Reconstruction Rules

After each merge:

1. Fetch `origin`.
2. Fast-forward local `develop`.
3. Re-evaluate the next branch with ancestry and patch-equivalence checks.
4. Create a clean branch from the updated `origin/develop`.
5. Apply only that branch's unique logical commits.
6. Resolve moved-path conflicts without changing runtime behavior.
7. Validate the exact candidate through a disposable Git bundle.
8. Open a focused PR into `develop`.

Do not merge current cumulative branch tips after their dependencies have
changed. Their old commits remain evidence and authorship references.

## Pull-Request Gate

The GitHub connector can read this repository but returned HTTP 403 for PR
creation. The authenticated in-app GitHub session is the approved write
surface. PR #6 proved that path. If authentication becomes unavailable:

- validated branches remain unmerged;
- no direct merge into `develop` substitutes for a PR;
- compare URLs and PR text may be prepared locally;
- branch deletion does not begin.

## Planned Deletions

Deletion is deferred until all applicable PRs merge and the results document
proves no unique useful work remains.

Expected eventual deletion candidates:

```text
backup/native-install-contract-before-rework
style/static-analysis-cleanup
refactor/repository-layout-source
refactor/repository-layout-support
cleanup/remove-obsolete-files
docs/repository-architecture
test-simplification/mochad-redux
```

The first branch is superseded. The remaining branches are deleted only after
their merged or reconstructed replacements preserve their useful changes.
Attached worktrees must be removed before their local branches.

Never delete:

```text
master
develop
upstream/master
```

No archival tag is currently justified. GitHub PR history and replacement
commits preserve the relevant ancestry. Reconsider only if reconstruction
leaves historically important, unmerged code.

## Stop Conditions

Stop and request review if:

- source/support layout conflicts reveal competing runtime changes;
- a historical file has unclear licensing or provenance;
- test-simplification material cannot be separated from stale native-install
  assumptions;
- validation needs production services;
- hardware transmission is required;
- any deletion would discard unique useful work.
