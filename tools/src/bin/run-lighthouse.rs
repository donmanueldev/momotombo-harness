//! Run repeatable Lighthouse profiles, calculate medians, and enforce budgets.

use agent_tools::{die, inline_json, now_iso, pretty_json, resolve, write_or_die};
use clap::{Parser, ValueEnum};
use serde_json::{json, Map, Value};
use std::os::unix::fs::PermissionsExt;
use std::path::{Path, PathBuf};
use std::process::Command;

const METRICS: [(&str, [&str; 3]); 7] = [
    ("performanceScore", ["categories", "performance", "score"]),
    ("firstContentfulPaintMs", ["audits", "first-contentful-paint", "numericValue"]),
    ("largestContentfulPaintMs", ["audits", "largest-contentful-paint", "numericValue"]),
    ("speedIndexMs", ["audits", "speed-index", "numericValue"]),
    ("totalBlockingTimeMs", ["audits", "total-blocking-time", "numericValue"]),
    ("cumulativeLayoutShift", ["audits", "cumulative-layout-shift", "numericValue"]),
    ("totalByteWeightBytes", ["audits", "total-byte-weight", "numericValue"]),
];

#[derive(Parser)]
#[command(about = "Run repeatable Lighthouse profiles, calculate medians, and enforce budgets.")]
struct Args {
    #[arg(long)]
    url: String,
    #[arg(long, value_enum, default_value_t = Profiles::Both)]
    profiles: Profiles,
    #[arg(long, default_value_t = 5)]
    runs: i64,
    #[arg(long, default_value = "lighthouse-audit")]
    output_dir: PathBuf,
    #[arg(long)]
    budget: Option<PathBuf>,
    #[arg(long)]
    baseline: Option<PathBuf>,
    #[arg(long)]
    lighthouse_bin: Option<String>,
    #[arg(long, default_value = ".")]
    project_root: PathBuf,
    #[arg(long, default_value = "--headless", allow_hyphen_values = true)]
    chrome_flags: String,
}

#[derive(Clone, Copy, ValueEnum)]
enum Profiles {
    Mobile,
    Desktop,
    Both,
}

fn is_metric(name: &str) -> bool {
    METRICS.iter().any(|(metric, _)| *metric == name)
}

fn is_executable(path: &Path) -> bool {
    path.metadata().map(|meta| meta.is_file() && meta.permissions().mode() & 0o111 != 0).unwrap_or(false)
}

fn expand_home(path: &str) -> PathBuf {
    match (path.strip_prefix("~/"), std::env::var_os("HOME")) {
        (Some(rest), Some(home)) => PathBuf::from(home).join(rest),
        _ => PathBuf::from(path),
    }
}

fn find_lighthouse(explicit: Option<&str>, project_root: &Path) -> PathBuf {
    let mut candidates = Vec::new();
    if let Some(path) = explicit {
        candidates.push(expand_home(path));
    }
    candidates.push(project_root.join("node_modules/.bin/lighthouse"));
    if let Some(found) = candidates.into_iter().find(|candidate| candidate.is_file()) {
        return resolve(&found);
    }
    if let Some(paths) = std::env::var_os("PATH") {
        if let Some(found) = std::env::split_paths(&paths).map(|dir| dir.join("lighthouse")).find(|p| is_executable(p)) {
            return found;
        }
    }
    die("Lighthouse was not found. Add a pinned project devDependency or pass --lighthouse-bin; this script never downloads or installs it implicitly.")
}

fn lighthouse_version(binary: &Path) -> String {
    match Command::new(binary).arg("--version").output() {
        Ok(output) => {
            let stdout = String::from_utf8_lossy(&output.stdout).trim().to_string();
            let text = if stdout.is_empty() { String::from_utf8_lossy(&output.stderr).trim().to_string() } else { stdout };
            if text.is_empty() { "unknown".into() } else { text }
        }
        Err(_) => "unknown".into(),
    }
}

fn git_revision(project_root: &Path) -> Value {
    match Command::new("git").arg("-C").arg(project_root).args(["rev-parse", "HEAD"]).output() {
        Ok(output) if output.status.success() => json!(String::from_utf8_lossy(&output.stdout).trim()),
        _ => Value::Null,
    }
}

fn extract_metrics(document: &Value) -> Map<String, Value> {
    METRICS
        .iter()
        .map(|(name, path)| {
            let value = path.iter().try_fold(document, |node, key| node.get(key));
            let number = value.and_then(Value::as_f64).map(Value::from).unwrap_or(Value::Null);
            (name.to_string(), number)
        })
        .collect()
}

fn median(mut values: Vec<f64>) -> Option<f64> {
    if values.is_empty() {
        return None;
    }
    values.sort_by(|a, b| a.total_cmp(b));
    let mid = values.len() / 2;
    Some(if values.len() % 2 == 1 { values[mid] } else { (values[mid - 1] + values[mid]) / 2.0 })
}

fn median_metrics(runs: &[Value]) -> Map<String, Value> {
    METRICS
        .iter()
        .map(|(name, _)| {
            let values: Vec<f64> = runs.iter().filter_map(|run| run["metrics"][*name].as_f64()).collect();
            (name.to_string(), median(values).map(Value::from).unwrap_or(Value::Null))
        })
        .collect()
}

fn validate_budget_rules(data: &Map<String, Value>, label: &str) -> Map<String, Value> {
    let mut unknown: Vec<&str> = data.keys().map(String::as_str).filter(|key| !is_metric(key)).collect();
    if !unknown.is_empty() {
        unknown.sort();
        die(format!("Unknown budget metric(s) in {label}: {}", unknown.join(", ")));
    }
    for (metric, rule) in data {
        let valid_shape = rule
            .as_object()
            .is_some_and(|rule| !rule.is_empty() && rule.keys().all(|key| key == "min" || key == "max"));
        if !valid_shape {
            die(format!("Budget for {metric} in {label} must contain only min and/or max."));
        }
        if !rule.as_object().unwrap().values().all(Value::is_number) {
            die(format!("Budget values for {metric} in {label} must be numbers."));
        }
    }
    data.clone()
}

fn load_budgets(path: Option<&Path>) -> Map<String, Value> {
    let mut budgets = Map::new();
    let Some(path) = path else { return budgets };
    let text = std::fs::read_to_string(path).unwrap_or_else(|error| die(format!("Could not read budget file: {error}")));
    let data: Value = serde_json::from_str(&text).unwrap_or_else(|error| die(format!("Budget file is not valid JSON: {error}")));
    let Some(data) = data.as_object() else { die("Budget file must contain a JSON object.") };
    if data.keys().all(|key| is_metric(key)) {
        budgets.insert("default".into(), Value::Object(validate_budget_rules(data, "default")));
        return budgets;
    }
    let mut unknown: Vec<&str> =
        data.keys().map(String::as_str).filter(|key| !["default", "mobile", "desktop"].contains(key)).collect();
    if !unknown.is_empty() {
        unknown.sort();
        die(format!("Unknown budget profile(s): {}", unknown.join(", ")));
    }
    for (profile, rules) in data {
        let Some(rules) = rules.as_object() else { die(format!("Budget profile {profile} must contain a JSON object.")) };
        budgets.insert(profile.clone(), Value::Object(validate_budget_rules(rules, profile)));
    }
    budgets
}

fn budget_for_profile(budgets: &Map<String, Value>, profile: &str) -> Map<String, Value> {
    let mut merged = Map::new();
    for key in ["default", profile] {
        if let Some(rules) = budgets.get(key).and_then(Value::as_object) {
            for (metric, rule) in rules {
                merged.insert(metric.clone(), rule.clone());
            }
        }
    }
    merged
}

fn evaluate_budgets(medians: &Map<String, Value>, budgets: &Map<String, Value>) -> Vec<Value> {
    budgets
        .iter()
        .map(|(metric, rule)| {
            let actual = medians.get(metric).and_then(Value::as_f64);
            let passed = actual.is_some_and(|value| {
                rule["min"].as_f64().is_none_or(|min| value >= min) && rule["max"].as_f64().is_none_or(|max| value <= max)
            });
            json!({"metric": metric, "actual": actual, "budget": rule, "passed": passed})
        })
        .collect()
}

fn run_profile(binary: &Path, url: &str, profile: &str, runs: i64, output_dir: &Path, chrome_flags: &str) -> Result<Value, String> {
    let profile_dir = output_dir.join("runs").join(profile);
    std::fs::create_dir_all(&profile_dir).map_err(|error| error.to_string())?;
    let mut results = Vec::new();
    for number in 1..=runs {
        let output_path = profile_dir.join(format!("run-{number}.json"));
        let mut command = Command::new(binary);
        command.args([
            url.to_string(),
            "--only-categories=performance".into(),
            "--output=json".into(),
            format!("--output-path={}", output_path.display()),
            "--quiet".into(),
            format!("--chrome-flags={chrome_flags}"),
        ]);
        if profile == "desktop" {
            command.arg("--preset=desktop");
        }
        let started = now_iso();
        let output = command.output().map_err(|error| error.to_string())?;
        if !output.status.success() {
            let stderr = String::from_utf8_lossy(&output.stderr);
            let detail = if stderr.is_empty() { String::from_utf8_lossy(&output.stdout) } else { stderr };
            return Err(format!(
                "Lighthouse {profile} run {number} failed with exit {}:\n{detail}",
                output.status.code().unwrap_or(-1)
            ));
        }
        let text = std::fs::read_to_string(&output_path).map_err(|error| error.to_string())?;
        let document: Value = serde_json::from_str(&text).map_err(|error| error.to_string())?;
        results.push(json!({
            "number": number,
            "startedAt": started,
            "fetchTime": document.get("fetchTime").cloned().unwrap_or(Value::Null),
            "finalUrl": document.get("finalUrl").cloned().unwrap_or(Value::Null),
            "configSettings": document.get("configSettings").cloned().unwrap_or(Value::Null),
            "metrics": extract_metrics(&document),
            "artifact": output_path.display().to_string(),
        }));
        println!("Completed {profile} run {number}/{runs}");
    }
    let median = median_metrics(&results);
    Ok(json!({"runs": results, "median": median}))
}

fn load_baseline(path: Option<&Path>) -> Option<Value> {
    let path = path?;
    let text = std::fs::read_to_string(path).unwrap_or_else(|error| die(format!("Could not read baseline: {error}")));
    let value: Value = serde_json::from_str(&text).unwrap_or_else(|error| die(format!("Baseline is not valid JSON: {error}")));
    if value["schemaVersion"] != json!(1) || !value["profiles"].is_object() {
        die("Baseline is not a compatible Lighthouse median report.");
    }
    Some(value)
}

fn add_deltas(report: &mut Value, baseline: Option<&Value>) {
    let Some(baseline) = baseline else { return };
    report["baseline"] = match &baseline["reportPath"] {
        Value::Null => json!("provided report"),
        Value::String(text) => json!(text),
        other => json!(other.to_string()),
    };
    let Some(profiles) = report["profiles"].as_object_mut() else { return };
    for (profile, result) in profiles.iter_mut() {
        let old = &baseline["profiles"][profile.as_str()]["median"];
        let deltas: Map<String, Value> = result["median"]
            .as_object()
            .into_iter()
            .flatten()
            .map(|(metric, value)| {
                let delta = match (value.as_f64(), old[metric.as_str()].as_f64()) {
                    (Some(new), Some(previous)) => Value::from(new - previous),
                    _ => Value::Null,
                };
                (metric.clone(), delta)
            })
            .collect();
        result["deltaFromBaseline"] = Value::Object(deltas);
    }
}

fn format_value(metric: &str, value: Option<f64>) -> String {
    let Some(value) = value else { return "n/a".into() };
    if metric == "performanceScore" {
        format!("{:.0}", value * 100.0)
    } else if metric.ends_with("Ms") {
        format!("{value:.0} ms")
    } else if metric.ends_with("Bytes") {
        format!("{:.1} KiB", value / 1024.0)
    } else {
        format!("{value:.3}")
    }
}

fn title_case(text: &str) -> String {
    let mut chars = text.chars();
    chars.next().map(|first| first.to_uppercase().chain(chars).collect()).unwrap_or_default()
}

fn markdown_report(report: &Value) -> String {
    let mut lines = vec![
        "# Lighthouse Median Report".to_string(),
        String::new(),
        format!("- URL: `{}`", report["url"].as_str().unwrap_or_default()),
        format!("- Lighthouse: `{}`", report["lighthouseVersion"].as_str().unwrap_or_default()),
        format!("- Runs per profile: `{}`", report["runsPerProfile"]),
        format!("- Generated: `{}`", report["generatedAt"].as_str().unwrap_or_default()),
        String::new(),
    ];
    for (profile, result) in report["profiles"].as_object().into_iter().flatten() {
        lines.extend([
            format!("## {}", title_case(profile)),
            String::new(),
            "| Metric | Median | Budget | Result |".into(),
            "| --- | ---: | --- | --- |".into(),
        ]);
        let budgets = result["budgetResults"].as_array().cloned().unwrap_or_default();
        for (metric, value) in result["median"].as_object().into_iter().flatten() {
            let budget = budgets.iter().find(|item| item["metric"] == json!(metric));
            let budget_text = budget.map(|item| inline_json(&item["budget"])).unwrap_or_else(|| "-".into());
            let outcome = match budget {
                Some(item) if item["passed"] == json!(true) => "pass",
                Some(_) => "fail",
                None => "-",
            };
            lines.push(format!("| `{metric}` | {} | `{budget_text}` | {outcome} |", format_value(metric, value.as_f64())));
        }
        if let Some(deltas) = result.get("deltaFromBaseline").and_then(Value::as_object) {
            lines.extend([String::new(), "### Delta from baseline".into(), String::new(), "| Metric | Delta |".into(), "| --- | ---: |".into()]);
            for (metric, value) in deltas {
                lines.push(format!("| `{metric}` | {} |", format_value(metric, value.as_f64())));
            }
        }
        lines.push(String::new());
    }
    let overall = if report["passed"] == json!(true) { "pass" } else { "fail" };
    lines.push(format!("Overall budget result: **{overall}**"));
    lines.push(String::new());
    lines.push("> Laboratory evidence is synthetic. Keep it separate from CrUX field data and real-user monitoring.".into());
    lines.push(String::new());
    lines.join("\n")
}

fn main() {
    let args = Args::parse();
    if args.runs < 1 {
        die("--runs must be at least 1");
    }
    let project_root = resolve(&args.project_root);
    let binary = find_lighthouse(args.lighthouse_bin.as_deref(), &project_root);
    if let Err(error) = std::fs::create_dir_all(&args.output_dir) {
        die(format!("Could not create {}: {error}", args.output_dir.display()));
    }
    let output_dir = resolve(&args.output_dir);
    let budgets = load_budgets(args.budget.as_deref());
    let baseline = load_baseline(args.baseline.as_deref());
    let profile_names: &[&str] = match args.profiles {
        Profiles::Both => &["mobile", "desktop"],
        Profiles::Mobile => &["mobile"],
        Profiles::Desktop => &["desktop"],
    };

    let mut profiles = Map::new();
    for profile in profile_names {
        let mut result = match run_profile(&binary, &args.url, profile, args.runs, &output_dir, &args.chrome_flags) {
            Ok(result) => result,
            Err(error) => {
                eprintln!("{error}");
                std::process::exit(1);
            }
        };
        let median = result["median"].as_object().cloned().unwrap_or_default();
        let budget_results = evaluate_budgets(&median, &budget_for_profile(&budgets, profile));
        let passed = budget_results.iter().all(|item| item["passed"] == json!(true));
        result["budgetResults"] = json!(budget_results);
        result["passed"] = json!(passed);
        profiles.insert(profile.to_string(), result);
    }
    let passed = profiles.values().all(|result| result["passed"] == json!(true));
    let report_path = output_dir.join("report.json");
    let mut report = json!({
        "schemaVersion": 1,
        "generatedAt": now_iso(),
        "url": args.url,
        "projectRoot": project_root.display().to_string(),
        "gitRevision": git_revision(&project_root),
        "lighthouseVersion": lighthouse_version(&binary),
        "chromeFlags": args.chrome_flags,
        "runsPerProfile": args.runs,
        "profiles": profiles,
        "passed": passed,
        "reportPath": report_path.display().to_string(),
    });
    add_deltas(&mut report, baseline.as_ref());
    write_or_die(&report_path, &pretty_json(&report));
    let markdown_path = output_dir.join("report.md");
    write_or_die(&markdown_path, &markdown_report(&report));
    println!("Wrote {}", report_path.display());
    println!("Wrote {}", markdown_path.display());
    std::process::exit(if passed { 0 } else { 2 });
}
