import io
import json
import os
import subprocess
import sys
import tarfile
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
COLLECTOR = ROOT / "scripts/support/collect_support_bundle.py"
SHA = "a" * 40


def input_document(log=None):
    document = {
        "component": {"sha": SHA, "ref": "develop", "dirty": False},
        "version": "0.5.0",
        "diagnostics": {
            "hello": {
                "ok": True,
                "name": "mochad-redux",
                "daemon": "mochad-redux",
                "version": "0.5.0",
                "upstream_base": "mochad 0.1.18",
                "diagnostics": True,
            },
            "health": {
                "ok": True,
                "name": "mochad-redux",
                "version": "0.5.0",
                "upstream_base": "mochad 0.1.18",
                "usb_connected": True,
                "controller": "CM19A",
                "bind_address": "192.168.8.99",
                "listeners": {"main": {"enabled": True, "port": 1099}},
            },
        },
    }
    if log is not None:
        document["log"] = log
    return document


class SupportBundleTests(unittest.TestCase):
    def run_collector(self, document, output, *extra):
        return subprocess.run(
            [sys.executable, str(COLLECTOR), "--output", str(output), *extra],
            input=json.dumps(document),
            text=True,
            capture_output=True,
            check=False,
        )

    def test_creates_sanitized_bounded_bundle(self):
        log = "\n".join([
            "listener 192.168.8.99 path=/srv/mochad",
            "Rx RF HouseUnit: A1 Func: On",
            "Rx RFSEC Addr: 01:23:45 Func: CONTACT_ALERT",
            "password=hunter2",
            "this line must be truncated",
        ])
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "support.tar.gz"
            result = self.run_collector(
                input_document(log), output, "--max-log-lines", "4"
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(os.stat(output).st_mode & 0o777, 0o600)

            with tarfile.open(output, "r:gz") as archive:
                names = archive.getnames()
                self.assertIn("manifest.json", names)
                self.assertIn("scan-result.json", names)
                self.assertTrue(all(
                    member.mode & 0o077 == 0
                    for member in archive.getmembers()
                    if member.isfile()
                ))
                self.assertTrue(all(
                    member.uid == 0 and member.gid == 0
                    and member.uname == "" and member.gname == ""
                    for member in archive.getmembers()
                ))
                self.assertTrue(all(
                    member.mode & 0o777 == 0o700
                    for member in archive.getmembers()
                    if member.isdir()
                ))
                manifest = json.load(archive.extractfile("manifest.json"))
                health = json.load(archive.extractfile("facts/health.json"))
                saved_log = archive.extractfile("logs/mochad.log").read().decode()

            self.assertEqual(manifest["schema_version"], 1)
            self.assertEqual(
                set(manifest), {
                    "bundle_id", "components", "created_at", "entries", "generator",
                    "redaction", "schema_version", "secret_scan",
                }
            )
            self.assertTrue(manifest["secret_scan"]["completed_archive"])
            self.assertEqual(manifest["secret_scan"]["status"], "PASS")
            self.assertEqual(
                manifest["generator"]["repository_sha"],
                SHA,
            )
            self.assertTrue(
                all(
                    entry["repository_owner"] == "mochad-redux"
                    for entry in manifest["entries"]
                )
            )
            self.assertEqual(health["bind_address"], "HOST_1")
            self.assertNotIn("192.168.8.99", saved_log)
            self.assertIn("DEVICE_1", saved_log)
            self.assertIn("[REDACTED:security_id]", saved_log)
            self.assertNotIn("hunter2", saved_log)
            self.assertIn("password=[REDACTED:secret]", saved_log)
            self.assertNotIn("this line must be truncated", saved_log)

    def test_unresolved_high_entropy_value_fails_closed(self):
        document = input_document(
            "opaque ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnop\n"
        )
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "support.tar.gz"
            result = self.run_collector(document, output)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("high_entropy_candidate", result.stderr)
            self.assertFalse(output.exists())

    def test_invalid_input_fails_without_archive(self):
        document = input_document()
        document["arbitrary_path"] = "/etc"
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "support.tar.gz"
            result = self.run_collector(document, output)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("unsupported input field", result.stderr)
            self.assertFalse(output.exists())

    def test_refuses_to_overwrite_existing_archive(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "support.tar.gz"
            output.write_text("keep")
            result = self.run_collector(input_document(), output)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_text(), "keep")

    def test_rejects_malformed_diagnostic_shape(self):
        document = input_document()
        document["diagnostics"]["health"] = ["not", "an", "object"]
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "support.tar.gz"
            result = self.run_collector(document, output)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("diagnostics.health must be an object", result.stderr)
            self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
