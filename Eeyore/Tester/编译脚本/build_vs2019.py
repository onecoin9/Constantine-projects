#!/usr/bin/env python3
"""
Build Visual Studio 2019 solution using Python.

This script locates MSBuild.exe (preferred) or devenv.com via vswhere and builds
the provided solution. It supports common parameters like configuration,
platform, target (Build/Rebuild/Clean), parallel build, and optional log file.

Example (PowerShell):
  python .\build_vs2019.py --solution doorPressureTester\\doorPressureTester.sln \
         --configuration Release --platform x64 --target Build --parallel 8 --log build.log

Requirements:
  - Visual Studio 2019 installed (version 16.x) [[uses vswhere if available]]
  - Windows
"""

from __future__ import annotations

import argparse
import os
import sys
import subprocess
import shutil
from datetime import datetime
from typing import Iterable, Optional, Sequence


def debug(msg: str) -> None:
    print(f"[build] {msg}")


def is_windows() -> bool:
    return os.name == "nt"


def default_solution_path() -> str:
    # Default to the repo layout in this workspace
    return os.path.join("doorPressureTester", "doorPressureTester.sln")


def find_vswhere() -> Optional[str]:
    # Prefer env var override
    env_path = os.environ.get("VSWHERE")
    if env_path and os.path.isfile(env_path):
        return env_path
    # Standard install path
    candidates = [
        os.path.join(os.environ.get("ProgramFiles(x86)", r"C:\\Program Files (x86)"),
                     "Microsoft Visual Studio", "Installer", "vswhere.exe"),
        os.path.join(os.environ.get("ProgramFiles", r"C:\\Program Files"),
                     "Microsoft Visual Studio", "Installer", "vswhere.exe"),
    ]
    for path in candidates:
        if os.path.isfile(path):
            return path
    return None


def run_checked(args: Sequence[str], cwd: Optional[str] = None) -> subprocess.CompletedProcess:
    debug("Executing: " + " ".join(f'"{a}"' if " " in a else a for a in args))
    return subprocess.run(args, cwd=cwd, check=True, capture_output=True, text=True)


def vswhere_find(pattern: str) -> Optional[str]:
    vswhere = find_vswhere()
    if not vswhere:
        return None
    # Search only VS2019 (16.x)
    args = [
        vswhere,
        "-latest",
        "-products", "*",
        "-version", "[16,17)",
        "-requires", "Microsoft.Component.MSBuild",
        "-find", pattern,
    ]
    try:
        cp = run_checked(args)
    except subprocess.CalledProcessError:
        return None
    path = cp.stdout.strip().splitlines()[0] if cp.stdout.strip() else None
    return path if path and os.path.exists(path) else None


def find_msbuild() -> Optional[str]:
    # Try vswhere first
    found = vswhere_find(r"MSBuild\\**\\Bin\\MSBuild.exe")
    if found:
        return found
    # Fallback to common locations (Community/Professional/Enterprise)
    base = os.environ.get("ProgramFiles(x86)", r"C:\\Program Files (x86)")
    editions = ["Community", "Professional", "Enterprise", "BuildTools"]
    for ed in editions:
        candidate = os.path.join(
            base,
            "Microsoft Visual Studio",
            "2019",
            ed,
            "MSBuild",
            "Current",
            "Bin",
            "MSBuild.exe",
        )
        if os.path.isfile(candidate):
            return candidate
    return shutil.which("MSBuild.exe")


def find_devenv() -> Optional[str]:
    # Try vswhere for devenv.com
    found = vswhere_find(r"Common7\\IDE\\devenv.com")
    if found:
        return found
    # Fallback to common locations
    base = os.environ.get("ProgramFiles(x86)", r"C:\\Program Files (x86)")
    editions = ["Community", "Professional", "Enterprise"]
    for ed in editions:
        candidate = os.path.join(
            base,
            "Microsoft Visual Studio",
            "2019",
            ed,
            "Common7",
            "IDE",
            "devenv.com",
        )
        if os.path.isfile(candidate):
            return candidate
    return shutil.which("devenv.com")


def tee_process_output(proc: subprocess.Popen, log_file: Optional[str]) -> int:
    log = None
    try:
        if log_file:
            os.makedirs(os.path.dirname(os.path.abspath(log_file)) or ".", exist_ok=True)
            log = open(log_file, "w", encoding="utf-8", newline="")
            header = f"# Build started at {datetime.now().isoformat()}\n"
            log.write(header)
        # Stream stdout/stderr
        assert proc.stdout is not None
        assert proc.stderr is not None
        while True:
            line = proc.stdout.readline()
            if not line:
                break
            sys.stdout.write(line)
            if log:
                log.write(line)
        err = proc.stderr.read()
        if err:
            sys.stderr.write(err)
            if log:
                log.write(err)
        return proc.wait()
    finally:
        if log:
            log.flush()
            log.close()


def build_with_msbuild(msbuild: str, solution: str, configuration: str, platform: str,
                       target: str, parallel: Optional[int], properties: Iterable[str],
                       log_file: Optional[str]) -> int:
    args = [
        msbuild,
        solution,
        "/nologo",
        "/v:minimal",
        f"/t:{target}",
        f"/p:Configuration={configuration}",
        f"/p:Platform={platform}",
    ]
    if parallel and parallel > 0:
        args.append(f"/m:{parallel}")
    for prop in properties:
        args.append(f"/p:{prop}")

    debug("Using MSBuild")
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return tee_process_output(proc, log_file)


def build_with_devenv(devenv: str, solution: str, configuration: str, platform: str,
                      target: str, log_file: Optional[str]) -> int:
    # devenv expects /Build "Config|Platform" (or /Rebuild, /Clean)
    target_flag = {
        "Build": "/Build",
        "Rebuild": "/Rebuild",
        "Clean": "/Clean",
    }.get(target, "/Build")

    args = [
        devenv,
        solution,
        target_flag,
        f"{configuration}|{platform}",
        "/Project", ""  # build entire solution
    ]

    debug("Using devenv.com fallback")
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return tee_process_output(proc, log_file)


def main() -> int:
    if not is_windows():
        print("This script only supports Windows.", file=sys.stderr)
        return 2

    parser = argparse.ArgumentParser(
        description="Build VS2019 solution (MSBuild preferred, devenv fallback)")
    parser.add_argument("--solution", default=default_solution_path(),
                        help="Path to .sln file (default: doorPressureTester/doorPressureTester.sln)")
    parser.add_argument("--configuration", default="Debug", choices=["Debug", "Release"],
                        help="Build configuration")
    parser.add_argument("--platform", default="x64", choices=["x64", "Win32"],
                        help="Build platform")
    parser.add_argument("--target", default="Build", choices=["Build", "Rebuild", "Clean"],
                        help="Build target")
    parser.add_argument("--parallel", type=int, default=os.cpu_count() or 8,
                        help="Max parallel build processes for MSBuild (/m)")
    parser.add_argument("--log", dest="log_file", default=None,
                        help="Optional log file path to tee build output")
    parser.add_argument("--no-ref-build", action="store_true",
                        help="Set BuildProjectReferences=false for MSBuild")

    args = parser.parse_args()

    solution_path = os.path.abspath(args.solution)
    if not os.path.isfile(solution_path):
        print(f"Solution not found: {solution_path}", file=sys.stderr)
        return 2

    msbuild = find_msbuild()
    if msbuild:
        extra_props = []  # type: list[str]
        if args.no_ref_build:
            extra_props.append("BuildProjectReferences=false")
        exit_code = build_with_msbuild(
            msbuild,
            solution_path,
            args.configuration,
            args.platform,
            args.target,
            args.parallel,
            extra_props,
            args.log_file,
        )
        return exit_code

    devenv = find_devenv()
    if devenv:
        return build_with_devenv(
            devenv,
            solution_path,
            args.configuration,
            args.platform,
            args.target,
            args.log_file,
        )

    print("Could not locate MSBuild.exe or devenv.com (VS2019). Please ensure Visual Studio 2019 is installed.",
          file=sys.stderr)
    vswhere = find_vswhere()
    if not vswhere:
        print("vswhere.exe not found. Install from https://github.com/microsoft/vswhere or ensure Visual Studio Installer is present.",
              file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())


