#!/usr/bin/env python3
"""Create an offline, sanitized mochad-redux support bundle."""

from __future__ import annotations

import argparse
import gzip
import hashlib
import ipaddress
import json
import os
import re
import sys
import tarfile
import tempfile
import uuid
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any

SCHEMA_VERSION = 1
GENERATOR_VERSION = "1"
MAX_INPUT_BYTES = 256 * 1024
DEFAULT_LOG_LINES = 200
DEFAULT_LOG_BYTES = 64 * 1024
MAX_LOG_LINES = 1000
MAX_LOG_BYTES = 1024 * 1024

ALLOWED_INPUT_FIELDS = {"component", "version", "diagnostics", "log"}
DIAGNOSTIC_SCHEMAS: dict[str, Any] = {
    "hello": {
        "ok": None,
        "name": None,
        "daemon": None,
        "version": None,
        "upstream_base": None,
        "diagnostics": None,
    },
    "capabilities": {
        "ok": None,
        "commands": [],
        "legacy_commands": [],
        "json": None,
        "single_line": None,
        "raw_data": None,
    },
    "health": {
        "ok": None,
        "name": None,
        "version": None,
        "upstream_base": None,
        "uptime_seconds": None,
        "usb_connected": None,
        "controller": None,
        "endpoints_ready": None,
        "transfers_ready": None,
        "clients_total": None,
        "usb_tx": {
            "out_completed": None,
            "ack_received": None,
            "ack_timeout": None,
            "unexpected_one_byte": None,
        },
        "bind_address": None,
        "listeners": {
            "main": {"enabled": None, "port": None},
            "xml": {"enabled": None, "port": None},
            "openremote": {"enabled": None, "port": None},
        },
    },
    "config": {
        "ok": None,
        "bind_address": None,
        "foreground": None,
        "raw_data": None,
        "dual_stack": None,
        "log_level": None,
        "listeners": {
            "main": {"enabled": None, "port": None},
            "xml": {"enabled": None, "port": None},
            "openremote": {"enabled": None, "port": None},
        },
    },
    "version": {
        "ok": None,
        "name": None,
        "daemon": None,
        "version": None,
        "upstream_base": None,
    },
}

SECRET_ASSIGNMENT_RE = re.compile(
    r"(?i)\b(password|passwd|token|secret|authorization|credential|"
    r"mqtt_password|mqtt_tls_key_password)\b(\s*[=:]\s*)([^\s,;]+)"
)
PEM_PRIVATE_RE = re.compile(
    r"-----BEGIN [^-]*PRIVATE KEY-----.*?-----END [^-]*PRIVATE KEY-----",
    re.DOTALL,
)
URL_USERINFO_RE = re.compile(r"(?i)\b([a-z][a-z0-9+.-]*://)[^/@\s]+@")
RFSEC_RE = re.compile(
    r"(?i)(RFSEC\s+Addr:\s*)(?:0x[0-9a-f]{2}|[0-9a-f]{2}(?::[0-9a-f]{2}){2})"
)
HOUSEUNIT_RE = re.compile(r"(?i)(HouseUnit:\s*)([A-P](?:1[0-6]|[1-9]))\b")
IPV4_RE = re.compile(r"(?<![0-9.])(?:\d{1,3}\.){3}\d{1,3}(?![0-9.])")
IPV6_CANDIDATE_RE = re.compile(r"(?<![0-9A-Fa-f:])(?:[0-9A-Fa-f]{0,4}:){2,7}"
                               r"[0-9A-Fa-f]{0,4}(?![0-9A-Fa-f:])")
PATH_RE = re.compile(r"(?<![\w.])/(?:[A-Za-z0-9._+-]+/)*[A-Za-z0-9._+-]+")
HOST_ASSIGNMENT_RE = re.compile(
    r"(?i)\b(host|hostname|client|peer|address|bind_address)"
    r"(\s*[=:]\s*)([A-Za-z0-9_.:-]+)"
)
USER_ASSIGNMENT_RE = re.compile(r"(?i)\b(user|username)(\s*[=:]\s*)([A-Za-z0-9_.-]+)")
HIGH_ENTROPY_RE = re.compile(r"(?<![A-Za-z0-9+/])[A-Za-z0-9+/=_-]{32,}(?![A-Za-z0-9+/])")

FILENAME_RULES = (
    ("environment_file", re.compile(r"(?i)(^|/)\.env(?:\.|$)")),
    ("credential_filename", re.compile(r"(?i)(password|passwd|token|secret|credential)")),
    ("private_key_filename", re.compile(r"(?i)(id_rsa|id_ed25519|\.key$|private[_-]?key)")),
)
CONTENT_RULES = (
    ("private_key", PEM_PRIVATE_RE),
    ("url_credentials", re.compile(r"(?i)\b[a-z][a-z0-9+.-]*://[^/@\s]+@")),
    ("raw_security_id", re.compile(
        r"(?i)RFSEC\s+Addr:\s*(?:0x[0-9a-f]{2}|[0-9a-f]{2}(?::[0-9a-f]{2}){2})"
    )),
    ("raw_x10_identity", re.compile(r"(?i)HouseUnit:\s*[A-P](?:1[0-6]|[1-9])\b")),
    ("raw_ipv4", IPV4_RE),
    ("secret_assignment", re.compile(
        r"(?i)\b(password|passwd|token|secret|authorization|credential|"
        r"mqtt_password|mqtt_tls_key_password)\b\s*[=:]\s*[\"']?"
        r"(?!\[REDACTED:secret\])[^\s,\"'}]{4,}"
    )),
)


class BundleError(ValueError):
    """An input or scan failure that must prevent archive publication."""


class Pseudonymizer:
    def __init__(self) -> None:
        self._values: dict[str, dict[str, str]] = {}

    def alias(self, kind: str, value: str) -> str:
        values = self._values.setdefault(kind, {})
        if value not in values:
            values[value] = f"{kind}_{len(values) + 1}"
        return values[value]

    def counts(self) -> dict[str, int]:
        return {kind: len(values) for kind, values in sorted(self._values.items())}


def _ruleset_hash() -> str:
    rules = {
        "diagnostic_schemas": DIAGNOSTIC_SCHEMAS,
        "filename": [(name, pattern.pattern) for name, pattern in FILENAME_RULES],
        "content": [(name, pattern.pattern) for name, pattern in CONTENT_RULES],
        "high_entropy": HIGH_ENTROPY_RE.pattern,
        "redaction": [
            SECRET_ASSIGNMENT_RE.pattern,
            PEM_PRIVATE_RE.pattern,
            URL_USERINFO_RE.pattern,
            RFSEC_RE.pattern,
            HOUSEUNIT_RE.pattern,
            IPV4_RE.pattern,
            IPV6_CANDIDATE_RE.pattern,
            PATH_RE.pattern,
            HOST_ASSIGNMENT_RE.pattern,
            USER_ASSIGNMENT_RE.pattern,
        ],
    }
    encoded = json.dumps(rules, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(encoded).hexdigest()


def _read_input(stream: Any) -> dict[str, Any]:
    raw = stream.buffer.read(MAX_INPUT_BYTES + 1)
    if len(raw) > MAX_INPUT_BYTES:
        raise BundleError(f"input exceeds {MAX_INPUT_BYTES} bytes")
    try:
        document = json.loads(raw)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise BundleError(f"input is not valid UTF-8 JSON: {exc}") from exc
    if not isinstance(document, dict):
        raise BundleError("input must be one JSON object")
    unknown = sorted(set(document) - ALLOWED_INPUT_FIELDS)
    if unknown:
        raise BundleError(f"unsupported input field(s): {', '.join(unknown)}")
    return document


def _validate_component(value: Any) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise BundleError("component must be an object")
    if set(value) != {"sha", "ref", "dirty"}:
        raise BundleError("component must contain only sha, ref, and dirty")
    sha = value["sha"]
    ref = value["ref"]
    dirty = value["dirty"]
    if not isinstance(sha, str) or re.fullmatch(r"[0-9a-f]{40}", sha) is None:
        raise BundleError("component.sha must be a full lowercase 40-character Git SHA")
    if not isinstance(ref, str) or not ref or len(ref) > 128:
        raise BundleError("component.ref must be a non-empty string of at most 128 characters")
    if not isinstance(dirty, bool):
        raise BundleError("component.dirty must be a boolean")
    return {"repository": "mochad-redux", "sha": sha, "ref": ref, "dirty": dirty}


def _validate_version(value: Any) -> str:
    if not isinstance(value, str) or re.fullmatch(
        r"\d+\.\d+\.\d+(?:-dev|-rc[1-9]\d*)?", value
    ) is None:
        raise BundleError("version must be a plain mochad-redux semantic version")
    return value


def _replace(pattern: re.Pattern[str], text: str, replacement: Any, redactions: set[str],
             name: str) -> str:
    replaced, count = pattern.subn(replacement, text)
    if count:
        redactions.add(name)
    return replaced


def _sanitize_text(text: str, aliases: Pseudonymizer, redactions: set[str]) -> str:
    text = _replace(PEM_PRIVATE_RE, text, "[REDACTED:private_key]", redactions, "private_key")
    text = _replace(
        SECRET_ASSIGNMENT_RE,
        text,
        lambda match: f"{match.group(1)}{match.group(2)}[REDACTED:secret]",
        redactions,
        "secret",
    )
    text = _replace(
        URL_USERINFO_RE,
        text,
        lambda match: f"{match.group(1)}[REDACTED:url_userinfo]@",
        redactions,
        "url_userinfo",
    )
    text = _replace(
        RFSEC_RE,
        text,
        lambda match: f"{match.group(1)}[REDACTED:security_id]",
        redactions,
        "security_id",
    )
    text = _replace(
        HOUSEUNIT_RE,
        text,
        lambda match: f"{match.group(1)}{aliases.alias('DEVICE', match.group(2).upper())}",
        redactions,
        "device_identity",
    )
    text = _replace(
        HOST_ASSIGNMENT_RE,
        text,
        lambda match: (
            f"{match.group(1)}{match.group(2)}{aliases.alias('HOST', match.group(3))}"
        ),
        redactions,
        "host_identity",
    )
    text = _replace(
        USER_ASSIGNMENT_RE,
        text,
        lambda match: (
            f"{match.group(1)}{match.group(2)}{aliases.alias('USER', match.group(3))}"
        ),
        redactions,
        "user_identity",
    )

    def replace_ipv4(match: re.Match[str]) -> str:
        try:
            ipaddress.ip_address(match.group(0))
        except ValueError:
            return match.group(0)
        redactions.add("host_identity")
        return aliases.alias("HOST", match.group(0))

    text = IPV4_RE.sub(replace_ipv4, text)

    def replace_ipv6(match: re.Match[str]) -> str:
        try:
            address = ipaddress.ip_address(match.group(0))
        except ValueError:
            return match.group(0)
        if address.version != 6:
            return match.group(0)
        redactions.add("host_identity")
        return aliases.alias("HOST", match.group(0))

    text = IPV6_CANDIDATE_RE.sub(replace_ipv6, text)
    text = _replace(
        PATH_RE,
        text,
        lambda match: aliases.alias("PATH", match.group(0)),
        redactions,
        "path",
    )
    return text


def _sanitize_value(value: Any, schema: Any, path: str, aliases: Pseudonymizer,
                    redactions: set[str]) -> Any:
    if isinstance(schema, dict):
        if not isinstance(value, dict):
            raise BundleError(f"{path} must be an object")
        unknown = sorted(set(value) - set(schema))
        if unknown:
            redactions.add("unsupported_fields")
        return {
            key: _sanitize_value(value[key], child, f"{path}.{key}", aliases, redactions)
            for key, child in schema.items()
            if key in value
        }
    if schema == []:
        if not isinstance(value, list) or not all(isinstance(item, str) for item in value):
            raise BundleError(f"{path} must be an array of strings")
        return [_sanitize_text(item, aliases, redactions) for item in value]
    if isinstance(value, (dict, list)):
        raise BundleError(f"{path} must be a scalar")
    if isinstance(value, str):
        key = path.rsplit(".", 1)[-1]
        if key in {"bind_address", "host", "hostname"}:
            redactions.add("host_identity")
            return aliases.alias("HOST", value)
        if key in {"user", "username"}:
            redactions.add("user_identity")
            return aliases.alias("USER", value)
        return _sanitize_text(value, aliases, redactions)
    return value


def _sanitize_diagnostics(
    value: Any, aliases: Pseudonymizer
) -> tuple[dict[str, Any], dict[str, set[str]]]:
    if value is None:
        return {}, {}
    if not isinstance(value, dict):
        raise BundleError("diagnostics must be an object")
    unknown = sorted(set(value) - set(DIAGNOSTIC_SCHEMAS))
    if unknown:
        raise BundleError(f"unsupported diagnostic input(s): {', '.join(unknown)}")
    sanitized = {}
    redactions = {}
    for name, document in value.items():
        redactions[name] = set()
        sanitized[name] = _sanitize_value(
            document,
            DIAGNOSTIC_SCHEMAS[name],
            f"diagnostics.{name}",
            aliases,
            redactions[name],
        )
    return sanitized, redactions


def _bounded_log(value: Any, max_lines: int, max_bytes: int, aliases: Pseudonymizer
                 ) -> tuple[str | None, set[str], bool]:
    if value is None:
        return None, set(), False
    if not isinstance(value, str):
        raise BundleError("log must be a string")
    encoded = value.encode("utf-8")
    truncated = len(encoded) > max_bytes
    bounded = encoded[:max_bytes].decode("utf-8", errors="ignore")
    lines = bounded.splitlines()
    if len(lines) > max_lines:
        lines = lines[:max_lines]
        truncated = True
    redactions: set[str] = set()
    sanitized = _sanitize_text("\n".join(lines), aliases, redactions)
    if sanitized:
        sanitized += "\n"
    return sanitized, redactions, truncated


def _write_file(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
    os.chmod(path.parent, 0o700)
    path.write_bytes(data)
    os.chmod(path, 0o600)


def _entry(logical_name: str, category: str, relative_path: str | None, data: bytes | None,
           sensitivity: str, redactions: set[str], status: str = "collected",
           reason: str = "") -> dict[str, Any]:
    return {
        "logical_name": logical_name,
        "repository_owner": "mochad-redux",
        "category": category,
        "collection_command": f"stdin:{logical_name}",
        "status": status,
        "path": relative_path,
        "media_type": (
            "application/json" if relative_path and relative_path.endswith(".json")
            else "text/plain"
        ),
        "size": len(data) if data is not None else 0,
        "sha256": hashlib.sha256(data).hexdigest() if data is not None else None,
        "sensitivity": sensitivity,
        "redactions": sorted(redactions),
        "reason": reason,
    }


def _scan_bytes(name: str, data: bytes) -> list[dict[str, str]]:
    findings: list[dict[str, str]] = []
    for rule, pattern in FILENAME_RULES:
        if pattern.search(name):
            findings.append({"file": name, "rule": rule})
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        return findings + [{"file": name, "rule": "binary_content"}]
    for rule, pattern in CONTENT_RULES:
        if pattern.search(text):
            findings.append({"file": name, "rule": rule})
    for match in IPV6_CANDIDATE_RE.finditer(text):
        try:
            if ipaddress.ip_address(match.group(0)).version == 6:
                findings.append({"file": name, "rule": "raw_ipv6"})
                break
        except ValueError:
            pass
    for match in HIGH_ENTROPY_RE.finditer(text):
        candidate = match.group(0)
        if re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", candidate):
            continue
        if re.fullmatch(
            r"[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}",
            candidate,
            re.IGNORECASE,
        ):
            continue
        if candidate.startswith("REDACTED"):
            continue
        findings.append({"file": name, "rule": "high_entropy_candidate"})
        break
    return findings


def _scan_stage(stage: Path) -> tuple[int, list[dict[str, str]]]:
    findings: list[dict[str, str]] = []
    files = 0
    for path in sorted(stage.rglob("*")):
        if path.is_symlink():
            findings.append({"file": path.relative_to(stage).as_posix(), "rule": "symlink"})
        elif path.is_file():
            files += 1
            findings.extend(_scan_bytes(path.relative_to(stage).as_posix(), path.read_bytes()))
    return files, findings


def _create_archive(stage: Path, archive: Path) -> None:
    with archive.open("wb") as raw:
        with gzip.GzipFile(filename="", mode="wb", fileobj=raw, mtime=0) as compressed:
            with tarfile.open(
                fileobj=compressed, mode="w", format=tarfile.PAX_FORMAT
            ) as output:
                for path in sorted(stage.rglob("*")):
                    info = output.gettarinfo(
                        str(path), arcname=path.relative_to(stage).as_posix()
                    )
                    info.uid = 0
                    info.gid = 0
                    info.uname = ""
                    info.gname = ""
                    info.mtime = 0
                    if info.isfile():
                        with path.open("rb") as source:
                            output.addfile(info, source)
                    else:
                        output.addfile(info)
    os.chmod(archive, 0o600)


def _scan_archive(archive: Path) -> tuple[int, list[dict[str, str]]]:
    findings: list[dict[str, str]] = []
    files = 0
    with tarfile.open(archive, "r:gz") as source:
        for member in source.getmembers():
            pure = PurePosixPath(member.name)
            if pure.is_absolute() or ".." in pure.parts:
                findings.append({"file": member.name, "rule": "unsafe_archive_path"})
                continue
            if member.issym() or member.islnk():
                findings.append({"file": member.name, "rule": "archive_link"})
                continue
            if not member.isfile():
                continue
            extracted = source.extractfile(member)
            if extracted is None:
                findings.append({"file": member.name, "rule": "unreadable_archive_member"})
                continue
            files += 1
            findings.extend(_scan_bytes(member.name, extracted.read()))
    return files, findings


def _json_bytes(value: Any) -> bytes:
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode()


def create_bundle(document: dict[str, Any], output: Path, max_log_lines: int,
                  max_log_bytes: int) -> dict[str, Any]:
    if output.exists():
        raise BundleError(f"refusing to overwrite existing output: {output}")
    if not output.parent.is_dir():
        raise BundleError(f"output directory does not exist: {output.parent}")
    filename_findings = _scan_bytes(output.name, b"")
    if filename_findings:
        raise BundleError(f"output filename failed secret scan: {filename_findings}")
    if max_log_lines < 1 or max_log_lines > MAX_LOG_LINES:
        raise BundleError(f"max log lines must be between 1 and {MAX_LOG_LINES}")
    if max_log_bytes < 1 or max_log_bytes > MAX_LOG_BYTES:
        raise BundleError(f"max log bytes must be between 1 and {MAX_LOG_BYTES}")

    component = _validate_component(document.get("component"))
    version = _validate_version(document.get("version"))
    component["version"] = version
    aliases = Pseudonymizer()
    diagnostics, diagnostic_redactions = _sanitize_diagnostics(
        document.get("diagnostics"), aliases
    )
    log, log_redactions, log_truncated = _bounded_log(
        document.get("log"), max_log_lines, max_log_bytes, aliases
    )
    ruleset_hash = _ruleset_hash()
    created_at = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

    with tempfile.TemporaryDirectory(prefix=".mochad-support-", dir=output.parent) as temp:
        stage = Path(temp) / "stage"
        stage.mkdir(mode=0o700)
        candidate = Path(temp) / "bundle.tar.gz"
        entries: list[dict[str, Any]] = []

        version_data = f"{version}\n".encode()
        _write_file(stage / "facts/version.txt", version_data)
        entries.append(_entry("version", "identity", "facts/version.txt", version_data,
                              "public", set()))

        for name in DIAGNOSTIC_SCHEMAS:
            if name not in diagnostics:
                entries.append(_entry(name, "diagnostic", None, None, "operational", set(),
                                      "omitted", "input not provided"))
                continue
            data = _json_bytes(diagnostics[name])
            relative = f"facts/{name}.json"
            _write_file(stage / relative, data)
            entries.append(_entry(name, "diagnostic", relative, data, "operational",
                                  diagnostic_redactions[name]))

        if log is None:
            entries.append(_entry("bounded_log", "log", None, None, "sensitive", set(),
                                  "omitted", "input not provided"))
        else:
            data = log.encode()
            reason = "input truncated to configured bounds" if log_truncated else ""
            _write_file(stage / "logs/mochad.log", data)
            entries.append(_entry("bounded_log", "log", "logs/mochad.log", data, "sensitive",
                                  log_redactions, "redacted" if log_redactions else "collected",
                                  reason))

        scan = {
            "scanner": "mochad-redux-support-scan",
            "version": GENERATOR_VERSION,
            "rule_set_sha256": ruleset_hash,
            "files_scanned": 0,
            "archive_members_scanned": 0,
            "findings_by_class": {},
            "unresolved_findings": 0,
            "completed_archive": False,
            "status": "PASS",
        }
        manifest = {
            "schema_version": SCHEMA_VERSION,
            "bundle_id": str(uuid.uuid4()),
            "created_at": created_at,
            "generator": {
                "name": "mochad-redux-support-bundle",
                "version": GENERATOR_VERSION,
                "repository_sha": component["sha"],
                "rule_set_sha256": ruleset_hash,
            },
            "components": [component],
            "redaction": {
                "format": "[REDACTED:<class>]",
                "aliases": aliases.counts(),
                "applied": sorted(
                    set().union(*diagnostic_redactions.values(), log_redactions)
                ),
            },
            "entries": entries,
            "secret_scan": scan,
        }

        _write_file(stage / "manifest.json", _json_bytes(manifest))
        _write_file(stage / "scan-result.json", _json_bytes(scan))
        files_scanned, findings = _scan_stage(stage)
        if findings:
            raise BundleError(f"secret scan failed before archive creation: {findings}")

        _create_archive(stage, candidate)
        archive_members, findings = _scan_archive(candidate)
        if findings:
            candidate.unlink(missing_ok=True)
            raise BundleError(f"secret scan failed for completed archive: {findings}")

        scan.update({
            "files_scanned": files_scanned,
            "archive_members_scanned": archive_members,
            "completed_archive": True,
        })
        manifest["secret_scan"] = scan
        _write_file(stage / "manifest.json", _json_bytes(manifest))
        _write_file(stage / "scan-result.json", _json_bytes(scan))

        _, findings = _scan_stage(stage)
        if findings:
            candidate.unlink(missing_ok=True)
            raise BundleError(f"secret scan failed after manifest finalization: {findings}")
        _create_archive(stage, candidate)
        _, findings = _scan_archive(candidate)
        if findings:
            candidate.unlink(missing_ok=True)
            raise BundleError(f"secret scan failed for final archive: {findings}")

        os.replace(candidate, output)
        os.chmod(output, 0o600)
        return manifest


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build an offline, sanitized mochad-redux support bundle from JSON on stdin."
    )
    parser.add_argument("--output", required=True, type=Path, help="new .tar.gz archive path")
    parser.add_argument("--max-log-lines", type=int, default=DEFAULT_LOG_LINES)
    parser.add_argument("--max-log-bytes", type=int, default=DEFAULT_LOG_BYTES)
    return parser


def main() -> int:
    args = _parser().parse_args()
    try:
        document = _read_input(sys.stdin)
        manifest = create_bundle(
            document, args.output, args.max_log_lines, args.max_log_bytes
        )
    except (BundleError, OSError, tarfile.TarError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print(f"PASS: created sanitized support bundle {args.output}")
    print(f"Bundle ID: {manifest['bundle_id']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
