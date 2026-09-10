#!/usr/bin/env python3
"""Run repeatable Lighthouse profiles, calculate medians, and enforce budgets."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
from typing import Any


METRICS = {
    "performanceScore": ("categories", "performance", "score"),
    "firstContentfulPaintMs": ("audits", "first-contentful-paint", "numericValue"),
    "largestContentfulPaintMs": ("audits", "largest-contentful-paint", "numericValue"),
    "speedIndexMs": ("audits", "speed-index", "numericValue"),
    "totalBlockingTimeMs": ("audits", "total-blocking-time", "numericValue"),
    "cumulativeLayoutShift": ("audits", "cumulative-layout-shift", "numericValue"),
    "totalByteWeightBytes": ("audits", "total-byte-weight", "numericValue"),
}


def nested(data: dict[str, Any], path: tuple[str, ...]) -> Any:
    value: Any = data
    for key in path:
        if not isinstance(value, dict) or key not in value:
            return None
        value = value[key]
    return value


def find_lighthouse(explicit: str | None, project_root: Path) -> str:
    candidates = []
    if explicit:
        candidates.append(Path(explicit).expanduser())
    candidates.append(project_root / "node_modules" / ".bin" / "lighthouse")
    for candidate in candidates:
        if candidate.is_file():
            return str(candidate.resolve())
    from_path = shutil.which("lighthouse")
    if from_path:
        return from_path
    raise SystemExit(
        "Lighthouse was not found. Add a pinned project devDependency or pass "
        "--lighthouse-bin; this script never downloads or installs it implicitly."
    )


def lighthouse_version(binary: str) -> str:
    result = subprocess.run([binary, "--version"], capture_output=True, text=True, check=False)
    return (result.stdout or result.stderr).strip() or "unknown"


def git_revision(project_root: Path) -> str | None:
    result = subprocess.run(["git", "-C", str(project_root), "rev-parse", "HEAD"], capture_output=True, text=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def extract_metrics(document: dict[str, Any]) -> dict[str, float | None]:
    values: dict[str, float | None] = {}
    for name, path in METRICS.items():
        value = nested(document, path)
        values[name] = float(value) if isinstance(value, (int, float)) else None
    return values


def median_metrics(runs: list[dict[str, Any]]) -> dict[str, float | None]:
    medians: dict[str, float | None] = {}
    for name in METRICS:
        values = [run["metrics"][name] for run in runs if run["metrics"].get(name) is not None]
        medians[name] = statistics.median(values) if values else None
    return medians


def validate_budget_rules(data: dict[str, Any], label: str) -> dict[str, Any]:
    unknown = set(data) - set(METRICS)
    if unknown:
        raise SystemExit(f"Unknown budget metric(s) in {label}: " + ", ".join(sorted(unknown)))
    for metric, rule in data.items():
        if not isinstance(rule, dict) or not rule or set(rule) - {"min", "max"}:
            raise SystemExit(f"Budget for {metric} in {label} must contain only min and/or max.")
        if not all(isinstance(value, (int, float)) for value in rule.values()):
            raise SystemExit(f"Budget values for {metric} in {label} must be numbers.")
    return data


def load_budgets(path: str | None) -> dict[str, dict[str, Any]]:
    if not path:
        return {}
    data = json.loads(Path(path).read_text())
    if not isinstance(data, dict):
        raise SystemExit("Budget file must contain a JSON object.")
    if set(data).issubset(METRICS):
        return {"default": validate_budget_rules(data, "default")}
    unknown_profiles = set(data) - {"default", "mobile", "desktop"}
    if unknown_profiles:
        raise SystemExit("Unknown budget profile(s): " + ", ".join(sorted(unknown_profiles)))
    budgets = {}
    for profile, rules in data.items():
        if not isinstance(rules, dict):
            raise SystemExit(f"Budget profile {profile} must contain a JSON object.")
        budgets[profile] = validate_budget_rules(rules, profile)
    return budgets


def budget_for_profile(budgets: dict[str, dict[str, Any]], profile: str) -> dict[str, Any]:
    return {**budgets.get("default", {}), **budgets.get(profile, {})}


def evaluate_budgets(medians: dict[str, float | None], budgets: dict[str, Any]) -> list[dict[str, Any]]:
    results = []
    for metric, rule in budgets.items():
        actual = medians.get(metric)
        passed = actual is not None
        if actual is not None and "min" in rule:
            passed = passed and actual >= rule["min"]
        if actual is not None and "max" in rule:
            passed = passed and actual <= rule["max"]
        results.append({"metric": metric, "actual": actual, "budget": rule, "passed": passed})
    return results


def run_profile(binary: str, url: str, profile: str, run_count: int, output_dir: Path, chrome_flags: str) -> dict[str, Any]:
    profile_dir = output_dir / "runs" / profile
    profile_dir.mkdir(parents=True, exist_ok=True)
    runs: list[dict[str, Any]] = []
    for number in range(1, run_count + 1):
        output_path = profile_dir / f"run-{number}.json"
        command = [
            binary,
            url,
            "--only-categories=performance",
            "--output=json",
            f"--output-path={output_path}",
            "--quiet",
            f"--chrome-flags={chrome_flags}",
        ]
        if profile == "desktop":
            command.append("--preset=desktop")
        started = datetime.now(timezone.utc).isoformat()
        result = subprocess.run(command, capture_output=True, text=True, check=False)
        if result.returncode != 0:
            raise RuntimeError(
                f"Lighthouse {profile} run {number} failed with exit {result.returncode}:\n"
                + (result.stderr or result.stdout)
            )
        document = json.loads(output_path.read_text())
        runs.append(
            {
                "number": number,
                "startedAt": started,
                "fetchTime": document.get("fetchTime"),
                "finalUrl": document.get("finalUrl"),
                "configSettings": document.get("configSettings"),
                "metrics": extract_metrics(document),
                "artifact": str(output_path),
            }
        )
        print(f"Completed {profile} run {number}/{run_count}")
    return {"runs": runs, "median": median_metrics(runs)}


def load_baseline(path: str | None) -> dict[str, Any] | None:
    if not path:
        return None
    value = json.loads(Path(path).read_text())
    if value.get("schemaVersion") != 1 or not isinstance(value.get("profiles"), dict):
        raise SystemExit("Baseline is not a compatible Lighthouse median report.")
    return value


def add_deltas(report: dict[str, Any], baseline: dict[str, Any] | None) -> None:
    if not baseline:
        return
    report["baseline"] = str(baseline.get("reportPath", "provided report"))
    for profile, result in report["profiles"].items():
        old = baseline.get("profiles", {}).get(profile, {}).get("median", {})
        result["deltaFromBaseline"] = {
            metric: (value - old[metric]) if value is not None and isinstance(old.get(metric), (int, float)) else None
            for metric, value in result["median"].items()
        }


def format_value(metric: str, value: float | None) -> str:
    if value is None:
        return "n/a"
    if metric == "performanceScore":
        return f"{value * 100:.0f}"
    if metric.endswith("Ms"):
        return f"{value:.0f} ms"
    if metric.endswith("Bytes"):
        return f"{value / 1024:.1f} KiB"
    return f"{value:.3f}"


def markdown_report(report: dict[str, Any]) -> str:
    lines = ["# Lighthouse Median Report", "", f"- URL: `{report['url']}`", f"- Lighthouse: `{report['lighthouseVersion']}`", f"- Runs per profile: `{report['runsPerProfile']}`", f"- Generated: `{report['generatedAt']}`", ""]
    for profile, result in report["profiles"].items():
        lines.extend([f"## {profile.title()}", "", "| Metric | Median | Budget | Result |", "| --- | ---: | --- | --- |"])
        budget_map = {item["metric"]: item for item in result["budgetResults"]}
        for metric, value in result["median"].items():
            budget = budget_map.get(metric)
            budget_text = json.dumps(budget["budget"]) if budget else "-"
            outcome = "pass" if budget and budget["passed"] else ("fail" if budget else "-")
            lines.append(f"| `{metric}` | {format_value(metric, value)} | `{budget_text}` | {outcome} |")
        if "deltaFromBaseline" in result:
            lines.extend(["", "### Delta from baseline", "", "| Metric | Delta |", "| --- | ---: |"])
            for metric, value in result["deltaFromBaseline"].items():
                lines.append(f"| `{metric}` | {format_value(metric, value)} |")
        lines.append("")
    lines.append(f"Overall budget result: **{'pass' if report['passed'] else 'fail'}**")
    lines.append("")
    lines.append("> Laboratory evidence is synthetic. Keep it separate from CrUX field data and real-user monitoring.")
    lines.append("")
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", required=True)
    parser.add_argument("--profiles", choices=("mobile", "desktop", "both"), default="both")
    parser.add_argument("--runs", type=int, default=5)
    parser.add_argument("--output-dir", default="lighthouse-audit")
    parser.add_argument("--budget")
    parser.add_argument("--baseline")
    parser.add_argument("--lighthouse-bin")
    parser.add_argument("--project-root", default=".")
    parser.add_argument("--chrome-flags", default="--headless")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")
    project_root = Path(args.project_root).resolve()
    binary = find_lighthouse(args.lighthouse_bin, project_root)
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    budgets = load_budgets(args.budget)
    profile_names = ["mobile", "desktop"] if args.profiles == "both" else [args.profiles]
    profiles = {}
    try:
        for profile in profile_names:
            result = run_profile(binary, args.url, profile, args.runs, output_dir, args.chrome_flags)
            result["budgetResults"] = evaluate_budgets(result["median"], budget_for_profile(budgets, profile))
            result["passed"] = all(item["passed"] for item in result["budgetResults"])
            profiles[profile] = result
    except (OSError, RuntimeError, json.JSONDecodeError) as error:
        print(str(error), file=sys.stderr)
        return 1
    report = {
        "schemaVersion": 1,
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "url": args.url,
        "projectRoot": str(project_root),
        "gitRevision": git_revision(project_root),
        "lighthouseVersion": lighthouse_version(binary),
        "chromeFlags": args.chrome_flags,
        "runsPerProfile": args.runs,
        "profiles": profiles,
        "passed": all(result["passed"] for result in profiles.values()),
    }
    report_path = output_dir / "report.json"
    report["reportPath"] = str(report_path)
    add_deltas(report, load_baseline(args.baseline))
    report_path.write_text(json.dumps(report, indent=2) + "\n")
    (output_dir / "report.md").write_text(markdown_report(report))
    print(f"Wrote {report_path}")
    print(f"Wrote {output_dir / 'report.md'}")
    return 0 if report["passed"] else 2


if __name__ == "__main__":
    sys.exit(main())
