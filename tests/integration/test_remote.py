#!/usr/bin/env python3
"""Docker-backed compatibility tests for a current geodepot repository."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import shlex
import subprocess
import sys
import tarfile
import tempfile
import uuid


VERSION = "1.2.0"
REQUIRED_INDEX_FIELDS = {
    "case_name",
    "data_name",
    "data_sha256",
    "data_size",
    "archive_sha256",
    "archive_size",
}


def command(args: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None,
            expected: int = 0) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(args, cwd=cwd, env=env, check=False, capture_output=True,
                            text=True, timeout=180)
    if result.returncode != expected:
        raise AssertionError(
            f"command returned {result.returncode}, expected {expected}: {shlex.join(args)}\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    return result


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(65536), b""):
            digest.update(block)
    return digest.hexdigest()


def producer_command(source: Path) -> list[str]:
    override = os.environ.get("GEODEPOT_EXECUTABLE")
    if override:
        return shlex.split(override)
    manifest = source / "pyproject.toml"
    if not manifest.is_file():
        raise RuntimeError(f"GEODEPOT_SOURCE has no pyproject.toml: {source}")
    pixi = shutil.which("pixi")
    if pixi is None:
        raise RuntimeError("pixi is required, or set GEODEPOT_EXECUTABLE")
    return [pixi, "run", "--manifest-path", str(manifest), "--executable", "geodepot"]


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def build_fixture(source: Path, fixture: Path, home: Path) -> tuple[list[str], dict[str, Path]]:
    sources = source / "tests" / "data" / "sources"
    entries = {
        "cityjson/item": sources / "3dbag_one.city.json",
        "cityjsonseq/item": sources / "tyler_debug" / "testfeature.city.jsonl",
        "ogr/item": sources / "wippolder" / "wippolder.gpkg",
        "gdal/item": sources / "wippolder" / "wippolder.tif",
        "pdal/item": sources / "wippolder" / "wippolder.las",
        "directory/tiles": sources / "tyler_debug" / "3dtiles",
    }
    missing = [str(path) for path in entries.values() if not path.exists()]
    if missing:
        raise RuntimeError("missing producer source data:\n" + "\n".join(missing))
    environment = os.environ.copy()
    environment["HOME"] = str(home)
    environment["XDG_CONFIG_HOME"] = str(home / ".config")
    geodepot = producer_command(source)
    command([*geodepot, "init"], cwd=fixture, env=environment)
    for casespec, source_path in entries.items():
        arguments = [*geodepot, "add"]
        if casespec == "cityjsonseq/item":
            arguments.extend(["--format", "cityjsonseq"])
        if source_path.is_dir():
            arguments.append("--as-data")
        command([*arguments, casespec, str(source_path)], cwd=fixture, env=environment)
    command([*geodepot, "check"], cwd=fixture, env=environment)

    index_path = fixture / ".geodepot" / "index.geojson"
    index = json.loads(index_path.read_text(encoding="utf-8"))
    features = index.get("features")
    if not isinstance(features, list) or len(features) != len(entries):
        raise AssertionError("producer did not create the complete fixture index")
    for feature in features:
        properties = feature.get("properties", {})
        if not REQUIRED_INDEX_FIELDS.issubset(properties):
            raise AssertionError(f"producer emitted an unsupported index schema: {properties}")
    return list(entries), entries


def write_release(fixture: Path, casespecs: list[str]) -> None:
    index = fixture / ".geodepot" / "index.geojson"
    write_json(fixture / "release.json", {
        "version": VERSION,
        "index_sha256": sha256(index),
        "status": "complete",
        "suites": {
            "all": {"casespecs": casespecs},
            "small": {"casespecs": casespecs[:2]},
        },
    })


def write_lock(path: Path, remote: str, fixture: Path) -> None:
    write_json(path, {
        "version": VERSION,
        "remote": remote,
        "index_sha256": sha256(fixture / ".geodepot" / "index.geojson"),
    })


def compose(compose_file: Path, project: str, environment: dict[str, str], *arguments: str,
            expected: int = 0) -> subprocess.CompletedProcess[str]:
    return command(["docker", "compose", "-f", str(compose_file), "-p", project, *arguments],
                   env=environment, expected=expected)


def endpoint(compose_file: Path, project: str, environment: dict[str, str]) -> str:
    value = compose(compose_file, project, environment, "port", "server", "80").stdout.strip()
    if not value:
        raise AssertionError("Docker did not publish the HTTP port")
    if value.startswith("0.0.0.0:"):
        value = "127.0.0.1:" + value.rsplit(":", 1)[1]
    return "http://" + value + "/geodepot"


def assert_payloads(root: Path, entries: dict[str, Path]) -> None:
    for casespec, source in entries.items():
        destination = root.joinpath(*casespec.split("/"))
        if source.is_file():
            if destination.read_bytes() != source.read_bytes():
                raise AssertionError(f"payload mismatch for {casespec}")
        else:
            expected = sorted(path.relative_to(source) for path in source.rglob("*") if path.is_file())
            actual = sorted(path.relative_to(destination) for path in destination.rglob("*") if path.is_file())
            if actual != expected:
                raise AssertionError(f"directory layout mismatch for {casespec}")
            for relative in expected:
                if (destination / relative).read_bytes() != (source / relative).read_bytes():
                    raise AssertionError(f"directory payload mismatch for {casespec}/{relative}")


def run_prepare(executable: Path, lock: Path, suite: str, cache: Path, *, offline: bool = False,
                expected: int = 0, probe: bool = False) -> subprocess.CompletedProcess[str]:
    if probe:
        return command([str(executable), str(lock), suite, str(cache)], expected=expected)
    arguments = [str(executable), "prepare", "--lock", str(lock), "--suite", suite, "--cache", str(cache)]
    if offline:
        arguments.append("--offline")
    return command(arguments, expected=expected)


def assert_no_parts(cache: Path) -> None:
    parts = list(cache.rglob("*.part")) if cache.exists() else []
    if parts:
        raise AssertionError(f"partial downloads remain: {parts}")


def replace_with_hostile_archive(fixture: Path, casespec: str) -> None:
    case, data = casespec.split("/", 1)
    archive = fixture / ".geodepot" / "cases" / case / f"{data}.tar"
    with tarfile.open(archive, "w") as output:
        member = tarfile.TarInfo(f"{data}/escape")
        member.type = tarfile.SYMTYPE
        member.linkname = "/etc/passwd"
        output.addfile(member)
    index_path = fixture / ".geodepot" / "index.geojson"
    index = json.loads(index_path.read_text(encoding="utf-8"))
    for feature in index["features"]:
        properties = feature["properties"]
        if properties["case_name"] == case and properties["data_name"] == data:
            properties["archive_sha256"] = sha256(archive)
            properties["archive_size"] = archive.stat().st_size
            break
    write_json(index_path, index)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--get", required=True, type=Path)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--source-root", required=True, type=Path)
    args = parser.parse_args()
    if not args.get.is_file() or not args.probe.is_file():
        raise RuntimeError("CMake did not build the integration executables")

    default_source = args.source_root.parent / "geodepot"
    source = Path(os.environ.get("GEODEPOT_SOURCE", default_source))
    if not source.is_dir():
        raise RuntimeError(f"geodepot producer is unavailable: {source}; set GEODEPOT_SOURCE")
    if shutil.which("docker") is None:
        raise RuntimeError("docker is required for the integration target")

    with tempfile.TemporaryDirectory(prefix="geodepot-api-integration-") as temporary:
        work = Path(temporary)
        fixture, home, cache = work / "fixture", work / "home", work / "cache"
        fixture.mkdir()
        home.mkdir()
        casespecs, entries = build_fixture(source, fixture, home)
        write_release(fixture, casespecs)
        environment = os.environ.copy()
        environment["GEODEPOT_SOURCE"] = str(source)
        environment["GEODEPOT_FIXTURE"] = str(fixture)
        compose_file = args.source_root / "tests" / "integration" / "docker-compose.yaml"
        project = "geodepot_api_" + uuid.uuid4().hex[:12]
        started = False
        try:
            compose(compose_file, project, environment, "up", "--detach", "--build")
            started = True
            remote = endpoint(compose_file, project, environment)
            lock = work / "geodepot.lock"
            write_lock(lock, remote, fixture)

            direct = run_prepare(args.probe, lock, "all", cache, probe=True)
            root = Path(direct.stdout.strip())
            if not root.is_absolute() or direct.stderr:
                raise AssertionError(f"library probe did not return only an absolute result path: stdout={direct.stdout!r}, stderr={direct.stderr!r}")
            assert_payloads(root, entries)
            again = run_prepare(args.get, lock, "all", cache)
            if Path(again.stdout.strip()) != root or again.stderr:
                raise AssertionError("CLI cache reuse did not return the prepared root cleanly")
            run_prepare(args.get, lock, "all", cache, offline=True)

            selected_cache = work / "selected-cache"
            selected = run_prepare(args.get, lock, "small", selected_cache)
            selected_root = Path(selected.stdout.strip())
            assert_payloads(selected_root, {key: entries[key] for key in casespecs[:2]})
            if any((selected_root / key.split("/", 1)[0]).exists() for key in casespecs[2:]):
                raise AssertionError("suite selection extracted data outside the selected suite")

            run_prepare(args.get, lock, "missing", work / "missing-suite", expected=3)
            run_prepare(args.get, lock, "all", work / "offline-miss", offline=True, expected=4)
            invalid = work / "invalid.lock"
            write_json(invalid, {"version": VERSION, "remote": "ssh://invalid", "index_sha256": "0" * 64})
            run_prepare(args.get, invalid, "all", work / "invalid-cache", expected=2)
            unreachable = work / "unreachable.lock"
            write_json(unreachable, {"version": VERSION, "remote": "http://127.0.0.1:9", "index_sha256": sha256(fixture / ".geodepot" / "index.geojson")})
            run_prepare(args.get, unreachable, "all", work / "unreachable-cache", expected=4)
            cache_file = work / "cache-file"
            cache_file.write_text("not a directory", encoding="utf-8")
            run_prepare(args.get, lock, "all", cache_file, expected=7)

            missing_archive = fixture / ".geodepot" / "cases" / casespecs[0].split("/", 1)[0] / f"{casespecs[0].split(chr(47), 1)[1]}.tar"
            archive_backup = missing_archive.read_bytes()
            missing_archive.unlink()
            run_prepare(args.get, lock, "small", work / "missing-artifact-cache", expected=3)
            missing_archive.write_bytes(archive_backup)

            concurrent_cache = work / "concurrent-cache"
            commands = [
                [str(args.get), "prepare", "--lock", str(lock), "--suite", "all", "--cache", str(concurrent_cache)],
                [str(args.get), "prepare", "--lock", str(lock), "--suite", "all", "--cache", str(concurrent_cache)],
            ]
            processes = [subprocess.Popen(item, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True) for item in commands]
            for process in processes:
                stdout, stderr = process.communicate(timeout=180)
                if process.returncode != 0:
                    raise AssertionError(f"concurrent preparation failed: {stdout}\n{stderr}")
            assert_no_parts(concurrent_cache)

            hostile_cache = work / "hostile-cache"
            replace_with_hostile_archive(fixture, casespecs[0])
            write_release(fixture, casespecs)
            write_lock(lock, remote, fixture)
            run_prepare(args.get, lock, "small", hostile_cache, expected=5)
            assert_no_parts(hostile_cache)

            index_path = fixture / ".geodepot" / "index.geojson"
            index = json.loads(index_path.read_text(encoding="utf-8"))
            index["features"][0]["properties"].pop("case_name")
            write_json(index_path, index)
            write_release(fixture, casespecs)
            write_lock(lock, remote, fixture)
            run_prepare(args.get, lock, "small", work / "schema-cache", expected=6)
        finally:
            if started:
                compose(compose_file, project, environment, "down", "--volumes", "--remove-orphans", expected=0)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, RuntimeError, subprocess.SubprocessError, OSError) as error:
        print(f"integration failure: {error}", file=sys.stderr)
        raise SystemExit(1)
