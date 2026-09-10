#!/usr/bin/env python3
"""Detect a React, Next.js, Astro, or static HTML audit adapter."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
from typing import Any


def package_manager(root: Path) -> tuple[str, list[str]]:
    if (root / "bun.lock").exists() or (root / "bun.lockb").exists():
        return "bun", ["bun", "run"]
    if (root / "pnpm-lock.yaml").exists():
        return "pnpm", ["pnpm", "run"]
    if (root / "yarn.lock").exists():
        return "yarn", ["yarn"]
    return "npm", ["npm", "run"]


def command(prefix: list[str], script: str, extra: list[str] | None = None) -> list[str]:
    return [*prefix, script, *(extra or [])]


def detect(root: Path, port: int) -> dict[str, Any]:
    package_file = root / "package.json"
    if not package_file.exists():
        index = root / "index.html"
        return {
            "framework": "static-html",
            "packageManager": None,
            "build": None,
            "serve": ["python3", "-m", "http.server", str(port), "--bind", "127.0.0.1", "--directory", str(root)],
            "outputDirectory": str(root),
            "url": f"http://127.0.0.1:{port}/",
            "notes": [] if index.exists() else ["No index.html found at the selected root."],
        }

    data = json.loads(package_file.read_text())
    scripts = data.get("scripts", {}) if isinstance(data.get("scripts"), dict) else {}
    dependencies = {}
    for key in ("dependencies", "devDependencies"):
        if isinstance(data.get(key), dict):
            dependencies.update(data[key])
    manager, run_prefix = package_manager(root)
    notes: list[str] = []
    build = command(run_prefix, "build") if "build" in scripts else None

    if "next" in dependencies:
        framework = "nextjs"
        output_directory = ".next"
        if "start" in scripts:
            serve = command(run_prefix, "start", ["--", "-p", str(port)])
        else:
            serve = None
            notes.append("No start script found; add a project-owned `next start` script. The adapter will not download tooling implicitly.")
    elif "astro" in dependencies:
        framework = "astro"
        output_directory = "dist"
        if "preview" in scripts:
            serve = command(run_prefix, "preview", ["--", "--host", "127.0.0.1", "--port", str(port)])
        else:
            serve = None
            notes.append("No preview script found; add `astro preview` or serve dist for a static output.")
    elif "react" in dependencies and "vite" in dependencies:
        framework = "react-vite"
        output_directory = "dist"
        if "preview" in scripts:
            serve = command(run_prefix, "preview", ["--", "--host", "127.0.0.1", "--port", str(port)])
        else:
            serve = None
            notes.append("No preview script found; add `vite preview` for production-build audits.")
    elif "react" in dependencies and "react-scripts" in dependencies:
        framework = "react-cra"
        output_directory = "build"
        serve = ["python3", "-m", "http.server", str(port), "--bind", "127.0.0.1", "--directory", str(root / output_directory)]
        notes.append("The static server does not emulate production rewrites or headers.")
    else:
        framework = "react-custom" if "react" in dependencies else "node-web"
        output_directory = None
        serve = command(run_prefix, "preview") if "preview" in scripts else (command(run_prefix, "start") if "start" in scripts else None)
        notes.append("Framework was not recognized; verify build output, production server, and port manually.")

    if build is None:
        notes.append("No build script found; do not substitute a development server for production evidence.")
    return {
        "framework": framework,
        "packageManager": manager,
        "build": build,
        "serve": serve,
        "outputDirectory": output_directory,
        "url": f"http://127.0.0.1:{port}/",
        "scripts": scripts,
        "notes": notes,
    }


def markdown(result: dict[str, Any]) -> str:
    def show(value: Any) -> str:
        if isinstance(value, list):
            return " ".join(json.dumps(part) if " " in part else part for part in value)
        return str(value or "manual")

    lines = ["# Web Audit Adapter", "", f"- Framework: `{result['framework']}`", f"- Package manager: `{result.get('packageManager') or 'none'}`", f"- Build: `{show(result.get('build'))}`", f"- Serve: `{show(result.get('serve'))}`", f"- Expected URL: `{result['url']}`"]
    if result.get("notes"):
        lines.extend(["", "## Verify", ""] + [f"- {note}" for note in result["notes"]])
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    parser.add_argument("--port", type=int, default=4173)
    parser.add_argument("--format", choices=("json", "markdown"), default="json")
    args = parser.parse_args()
    root = Path(args.root).resolve()
    try:
        result = detect(root, args.port)
    except (OSError, json.JSONDecodeError) as error:
        print(f"Could not inspect project: {error}", file=sys.stderr)
        return 1
    print(json.dumps(result, indent=2) if args.format == "json" else markdown(result), end="\n" if args.format == "json" else "")
    return 0


if __name__ == "__main__":
    sys.exit(main())
