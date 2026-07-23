## Summary

- 

## Scope

- [ ] Bug fix
- [ ] Documentation
- [ ] Build or CI maintenance
- [ ] Runtime diagnostics
- [ ] Hardware validation

## Release Branch Check

- [ ] This PR targets `develop`, or it is a release PR from `develop` into `master`.
- [ ] This PR does not tag a release from `develop`.

## Compatibility

- [ ] Existing mochad TCP behavior is preserved.
- [ ] No broad USB, TCP, protocol, or state refactor is included.
- [ ] User-facing logs are clear and do not expose secrets.

## Validation

- [ ] `sh scripts/build/compile-without-libusb.sh --strict --asan --ubsan`
- [ ] `sh scripts/build/compile-without-libusb.sh --cppcheck`
- [ ] `git diff --check`
- [ ] Full libusb build on Linux, if available
- [ ] CM19A hardware test, if available
- [ ] CM15A hardware test, if available
- [ ] Release evidence updated, if this is a release PR
- [ ] Generated artifacts reviewed against `docs/generated-artifacts.md`

## Notes
