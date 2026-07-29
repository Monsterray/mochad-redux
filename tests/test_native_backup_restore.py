#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tarfile
import tempfile
import textwrap
import unittest

REPOSITORY = pathlib.Path(__file__).resolve().parents[1]
TOOL = REPOSITORY / "scripts/backup/mochad-redux-backup"
git = shutil.which("git")
if git:
    git_sha = subprocess.run(
        [git, "rev-parse", "HEAD"],
        cwd=REPOSITORY,
        text=True,
        capture_output=True,
        check=False,
    )
    TEST_SHA = git_sha.stdout.strip() if git_sha.returncode == 0 else "1" * 40
else:
    TEST_SHA = "1" * 40
SYSTEMD = "/etc/systemd/system/mochad.service"
UDEV = "/etc/udev/rules.d/91-usb-x10-controllers.rules"
STATE = "/var/lib/mochad-redux/managed-files"
CONFIG = "/etc/mochad-redux/mochad.conf"


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def under(root: pathlib.Path, absolute: str) -> pathlib.Path:
    return root.joinpath(*pathlib.PurePosixPath(absolute).parts[1:])


class NativeBackupRestoreTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="redux-backup-test-")
        self.base = pathlib.Path(self.temporary.name)
        self.source = self.base / "source"
        self.restore = self.base / "restore"
        self.archive = self.base / "backup.tar.gz"
        self.source.mkdir()
        self.restore.mkdir()
        self.config_bytes = (
            b"bind=127.0.0.1\n"
            b"password=do-not-archive\n"
            b"endpoint=https://operator:credential@example.test/\n"
        )
        self.systemd_bytes = b"[Service]\nUser=mochad\n"
        self.udev_bytes = b'SUBSYSTEM=="usb", GROUP="x10"\n'
        self.write(self.source, CONFIG, self.config_bytes, 0o640)
        self.write(self.source, SYSTEMD, self.systemd_bytes)
        self.write(self.source, UDEV, self.udev_bytes)
        state = f"{SYSTEMD}|{digest(self.systemd_bytes)}\n{UDEV}|{digest(self.udev_bytes)}\n"
        self.write(self.source, STATE, state.encode())
        self.setup_tool = self.make_setup_tool(fail=False)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    @staticmethod
    def write(root: pathlib.Path, absolute: str, data: bytes, mode: int = 0o644) -> pathlib.Path:
        path = under(root, absolute)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        os.chmod(path, mode)
        return path

    def run_tool(
        self,
        *arguments: str,
        expected: int = 0,
        env: dict[str, str] | None = None,
    ) -> subprocess.CompletedProcess[str]:
        result = subprocess.run(
            [str(TOOL), *arguments],
            cwd=REPOSITORY,
            text=True,
            capture_output=True,
            check=False,
            env=env,
        )
        self.assertEqual(
            result.returncode,
            expected,
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}",
        )
        return result

    def create(self, root: pathlib.Path | None = None, *extra: str) -> None:
        self.run_tool(
            "create",
            "--root",
            str(root or self.source),
            "--output",
            str(self.archive),
            "--repository",
            str(REPOSITORY),
            "--repository-sha",
            TEST_SHA,
            *extra,
        )

    def manifest_and_payload(self) -> tuple[dict[str, object], bytes]:
        with tarfile.open(self.archive, "r:gz") as archive:
            manifest = json.load(archive.extractfile("manifest.json"))
            payload = archive.extractfile("payload/etc/mochad-redux/mochad.conf").read()
        return manifest, payload

    def rewrite_archive(self, *, schema: int | None = None, payload: bytes | None = None) -> None:
        manifest, current_payload = self.manifest_and_payload()
        if schema is not None:
            manifest["schema_version"] = schema
        replacement = self.base / "replacement.tar.gz"
        with tarfile.open(replacement, "w:gz") as archive:
            manifest_data = (json.dumps(manifest) + "\n").encode()
            for name, data in (
                ("manifest.json", manifest_data),
                ("payload/etc/mochad-redux/mochad.conf", payload or current_payload),
            ):
                info = tarfile.TarInfo(name)
                info.size = len(data)
                archive.addfile(info, __import__("io").BytesIO(data))
        os.replace(replacement, self.archive)

    def make_setup_tool(self, *, fail: bool) -> pathlib.Path:
        path = self.base / ("setup-fail.py" if fail else "setup.py")
        script = f"""\
            #!/usr/bin/env python3
            import hashlib
            import pathlib
            import sys

            args = sys.argv[1:]
            root = pathlib.Path(args[args.index("--root") + 1])
            systemd = {self.systemd_bytes!r}
            udev = {self.udev_bytes!r}
            records = []
            (root / "setup-arguments.txt").write_text(" ".join(args), encoding="utf-8")
            if "--no-systemd" not in args:
                path = root / {SYSTEMD[1:]!r}
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(systemd if not {fail!r} else b"partial-unit")
                records.append(str(path) + "|" + hashlib.sha256(systemd).hexdigest())
            if "--no-udev" not in args:
                path = root / {UDEV[1:]!r}
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(udev if not {fail!r} else b"partial-rule")
                records.append(str(path) + "|" + hashlib.sha256(udev).hexdigest())
            state = root / {STATE[1:]!r}
            state.parent.mkdir(parents=True, exist_ok=True)
            state.write_text("\\n".join(records) + ("\\n" if records else ""), encoding="utf-8")
            raise SystemExit(9 if {fail!r} else 0)
        """
        path.write_text(textwrap.dedent(script), encoding="utf-8")
        os.chmod(path, 0o755)
        return path

    def make_real_setup_tool(self) -> pathlib.Path:
        path = self.base / "mochad-redux-setup"
        content = (REPOSITORY / "scripts/setup/mochad-redux-setup.in").read_text(encoding="utf-8")
        replacements = {
            "@prefix@": "/usr/local",
            "@bindir@": "/usr/local/bin",
            "@pkgdatadir@": "/usr/local/share/mochad-redux",
        }
        for placeholder, value in replacements.items():
            content = content.replace(placeholder, value)
        path.write_text(content, encoding="utf-8")
        os.chmod(path, 0o755)
        return path

    @staticmethod
    def install_setup_templates(root: pathlib.Path) -> None:
        templates = root / "usr/local/share/mochad-redux/templates"
        templates.mkdir(parents=True, exist_ok=True)
        for source, name in (
            (REPOSITORY / "packaging/linux/systemd/mochad.service.in", "mochad.service.in"),
            (
                REPOSITORY / "packaging/linux/udev/91-usb-x10-controllers.rules.in",
                "91-usb-x10-controllers.rules.in",
            ),
            (REPOSITORY / "packaging/linux/config/mochad.conf.example", "mochad.conf.example"),
        ):
            (templates / name).write_bytes(source.read_bytes())

    def test_create_records_scope_and_excludes_credentials(self) -> None:
        self.create()
        manifest, payload = self.manifest_and_payload()
        self.assertEqual(manifest["schema_version"], 1)
        self.assertEqual(manifest["source"]["sha"], TEST_SHA)
        self.assertIn(b"${EXTERNAL_SECRET:password}", payload)
        self.assertNotIn(b"do-not-archive", payload)
        self.assertIn(b"${EXTERNAL_SECRET:endpoint}", payload)
        self.assertNotIn(b"operator:credential", payload)
        self.assertEqual([entry["role"] for entry in manifest["entries"]], ["native_config"])
        self.assertEqual(
            {record["kind"] for record in manifest["managed_integrations"]["records"]},
            {"systemd", "udev"},
        )
        with tarfile.open(self.archive, "r:gz") as archive:
            self.assertEqual(
                set(archive.getnames()),
                {"manifest.json", "payload/etc/mochad-redux/mochad.conf"},
            )
        self.assertEqual(self.archive.stat().st_mode & 0o777, 0o600)

    def test_create_uses_explicit_sha_when_git_is_unavailable(self) -> None:
        executable_directory = self.base / "bin"
        executable_directory.mkdir()
        (executable_directory / "python3").symlink_to(sys.executable)
        self.run_tool(
            "create",
            "--root",
            str(self.source),
            "--output",
            str(self.archive),
            "--repository",
            str(REPOSITORY),
            "--repository-sha",
            TEST_SHA,
            env={**os.environ, "PATH": str(executable_directory)},
        )
        self.assertEqual(self.manifest_and_payload()[0]["source"]["sha"], TEST_SHA)

    def test_invalid_schema_and_checksum_fail_closed(self) -> None:
        self.create()
        self.rewrite_archive(schema=2)
        result = self.run_tool("inspect", str(self.archive), expected=1)
        self.assertIn("unsupported backup schema", result.stderr)
        self.archive.unlink()
        self.create()
        self.rewrite_archive(payload=b"tampered")
        result = self.run_tool("inspect", str(self.archive), expected=1)
        self.assertIn("checksum mismatch", result.stderr)

    def test_managed_state_without_required_config_fails(self) -> None:
        source = self.base / "missing-config"
        source.mkdir()
        self.write(source, SYSTEMD, self.systemd_bytes)
        state = f"{SYSTEMD}|{digest(self.systemd_bytes)}\n"
        self.write(source, STATE, state.encode())
        result = self.run_tool(
            "create",
            "--root",
            str(source),
            "--output",
            str(self.archive),
            "--repository",
            str(REPOSITORY),
            "--repository-sha",
            TEST_SHA,
            expected=1,
        )
        self.assertIn("required configuration is missing", result.stderr)

    def test_private_key_material_is_refused(self) -> None:
        source = self.base / "private-key"
        source.mkdir()
        self.write(
            source,
            CONFIG,
            b"-----BEGIN PRIVATE KEY-----\nnot-a-real-key\n-----END PRIVATE KEY-----\n",
        )
        result = self.run_tool(
            "create",
            "--root",
            str(source),
            "--output",
            str(self.archive),
            "--repository",
            str(REPOSITORY),
            "--repository-sha",
            TEST_SHA,
            expected=1,
        )
        self.assertIn("private key material", result.stderr)

    def test_restore_is_dry_run_by_default(self) -> None:
        self.create()
        result = self.run_tool("restore", str(self.archive), "--root", str(self.restore))
        self.assertIn("dry_run=PASS apply=NOT RUN", result.stdout)
        self.assertFalse(under(self.restore, CONFIG).exists())
        self.assertFalse((self.restore / ".mochad-redux-restore-stage").exists())

    def test_apply_with_managed_records_requires_setup_tool(self) -> None:
        self.create()
        result = self.run_tool(
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            expected=1,
        )
        self.assertIn("--setup-tool is required", result.stderr)
        self.assertFalse(under(self.restore, CONFIG).exists())

    def test_apply_reconstructs_managed_files_and_is_idempotent(self) -> None:
        self.create()
        command = (
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            "--setup-tool",
            str(self.setup_tool),
        )
        self.run_tool(*command)
        restored = under(self.restore, CONFIG)
        self.assertEqual(restored.read_bytes(), self.manifest_and_payload()[1])
        self.assertEqual(restored.stat().st_mode & 0o777, 0o640)
        self.assertEqual(under(self.restore, SYSTEMD).read_bytes(), self.systemd_bytes)
        self.assertEqual(under(self.restore, UDEV).read_bytes(), self.udev_bytes)
        self.run_tool(*command)

    def test_overwrite_requires_explicit_approval(self) -> None:
        self.create()
        target = self.write(self.restore, CONFIG, b"local configuration\n")
        result = self.run_tool(
            "restore", str(self.archive), "--root", str(self.restore), "--apply", expected=1
        )
        self.assertIn("use --overwrite", result.stderr)
        self.assertEqual(target.read_bytes(), b"local configuration\n")
        self.run_tool(
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            "--overwrite",
            "--setup-tool",
            str(self.setup_tool),
        )

    def test_setup_failure_rolls_back_every_tracked_file(self) -> None:
        self.create()
        original = {
            CONFIG: b"old config\n",
            SYSTEMD: b"old unit\n",
            UDEV: b"old rule\n",
            STATE: b"old state\n",
        }
        for path, data in original.items():
            self.write(self.restore, path, data)
        failing_tool = self.make_setup_tool(fail=True)
        self.run_tool(
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            "--overwrite",
            "--setup-tool",
            str(failing_tool),
            expected=1,
        )
        for path, data in original.items():
            self.assertEqual(under(self.restore, path).read_bytes(), data)
        self.assertFalse((self.restore / ".mochad-redux-restore-stage").exists())

    def test_stale_stage_is_removed_before_apply(self) -> None:
        clean_source = self.base / "clean-source"
        clean_source.mkdir()
        self.write(clean_source, CONFIG, b"port=1099\n")
        self.create(clean_source)
        stale = self.restore / ".mochad-redux-restore-stage"
        stale.mkdir()
        (stale / "stale").write_text("stale", encoding="utf-8")
        self.run_tool("restore", str(self.archive), "--root", str(self.restore), "--apply")
        self.assertFalse(stale.exists())

    def test_absent_managed_state_needs_no_setup_tool(self) -> None:
        clean_source = self.base / "no-integrations"
        clean_source.mkdir()
        self.write(clean_source, CONFIG, b"raw_data=false\n")
        self.create(clean_source)
        manifest, _ = self.manifest_and_payload()
        self.assertEqual(manifest["managed_integrations"]["records"], [])
        self.run_tool("restore", str(self.archive), "--root", str(self.restore), "--apply")
        self.assertFalse(under(self.restore, SYSTEMD).exists())
        self.assertFalse(under(self.restore, UDEV).exists())

    def test_single_integration_disables_the_other_setup_path(self) -> None:
        state = f"{SYSTEMD}|{digest(self.systemd_bytes)}\n"
        self.write(self.source, STATE, state.encode())
        self.create()
        self.run_tool(
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            "--setup-tool",
            str(self.setup_tool),
        )
        arguments = (self.restore / "setup-arguments.txt").read_text(encoding="utf-8")
        self.assertIn("--no-udev", arguments)
        self.assertNotIn("--no-systemd", arguments)
        self.assertFalse(under(self.restore, UDEV).exists())

    def test_root_prefixed_managed_state_is_normalized(self) -> None:
        state = (
            f"{under(self.source, SYSTEMD)}|{digest(self.systemd_bytes)}\n"
            f"{under(self.source, UDEV)}|{digest(self.udev_bytes)}\n"
        )
        self.write(self.source, STATE, state.encode())
        self.create()
        manifest, _ = self.manifest_and_payload()
        self.assertEqual(
            [record["original_path"] for record in manifest["managed_integrations"]["records"]],
            [SYSTEMD, UDEV],
        )

    def test_real_setup_reconstruction_is_root_portable(self) -> None:
        setup_tool = self.make_real_setup_tool()
        self.install_setup_templates(self.source)
        self.install_setup_templates(self.restore)
        for path in (CONFIG, SYSTEMD, UDEV, STATE):
            under(self.source, path).unlink()
        subprocess.run(
            [str(setup_tool), "install", "--root", str(self.source)],
            check=True,
            text=True,
            capture_output=True,
        )

        self.create()
        self.run_tool(
            "restore",
            str(self.archive),
            "--root",
            str(self.restore),
            "--apply",
            "--setup-tool",
            str(setup_tool),
        )
        source_unit = under(self.source, SYSTEMD).read_text(encoding="utf-8")
        restored_unit = under(self.restore, SYSTEMD).read_text(encoding="utf-8")
        self.assertIn(str(self.source), source_unit)
        self.assertIn(str(self.restore.resolve()), restored_unit)
        self.assertEqual(
            source_unit.replace(str(self.source), ""),
            restored_unit.replace(str(self.restore.resolve()), ""),
        )

    def test_host_root_restore_is_refused(self) -> None:
        self.create()
        result = self.run_tool("restore", str(self.archive), "--root", "/", expected=1)
        self.assertIn("refuses host root", result.stderr)


if __name__ == "__main__":
    unittest.main()
