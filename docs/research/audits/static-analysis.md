# Static Analysis Audit

This records the initial cleanup pass for the maintained source tree.

## C Formatting

The repository now owns its formatting contract in
[`.clang-format`](../../../.clang-format).
It applies to maintained C and header files. The development-only libusb stub at
`tests/support/libusb-1.0/libusb.h` is excluded because it mirrors an external
interface and is not maintained as project code.

Formatting was applied as a mechanical-only commit. It must not be combined with
behavior changes.

## ShellCheck Disposition

| Finding | Disposition | Resolution |
| --- | --- | --- |
| `autogen.sh` unchecked directory change (`SC2164`) | Real defect | Check the directory change and stop on failure. |
| `autogen.sh` unsafe `$*` invocation (`SC2048`) | Real defect | Invoke the original argument vector with `"$@"`. |
| `CDPATH= cd` command substitutions (`SC1007`) | Robustness issue | Use `CDPATH=''` to prevent user environment output from corrupting paths. |
| Legacy shell client input/output quoting | Robustness issue | Use `IFS= read -r`, `printf`, and quoted case input. |
| `packaging/openwrt/init.d/mochad` ordering variable (`SC2034`) | Required legacy interface | Keep a narrow comment because hotplug2 reads `START` externally. |

No broad ShellCheck suppressions are accepted for maintained scripts. The CI
gate evaluates tracked shell sources at warning severity. Informational and
style-level diagnostics remain review input, not a reason to hide a defect.

## Local Checks

```sh
scripts/validate/clang-format.sh
scripts/validate/shellcheck.sh
```
