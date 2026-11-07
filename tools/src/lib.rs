//! Shared helpers for the agent-config CLI tools.

use serde_json::Value;
use std::path::{Component, Path, PathBuf};

/// Print `message` to stderr and exit with status 1.
pub fn die(message: impl std::fmt::Display) -> ! {
    eprintln!("{message}");
    std::process::exit(1);
}

/// Current UTC time as ISO 8601 with microseconds and a `+00:00` offset.
pub fn now_iso() -> String {
    chrono::Utc::now().to_rfc3339_opts(chrono::SecondsFormat::Micros, false)
}

/// Absolute, symlink-resolved path when it exists; lexically normalized absolute path otherwise.
pub fn resolve(path: &Path) -> PathBuf {
    if let Ok(real) = std::fs::canonicalize(path) {
        return real;
    }
    let absolute = std::path::absolute(path).unwrap_or_else(|_| path.to_path_buf());
    normalize_lexically(&absolute)
}

/// Collapse `.` and `..` components without touching the filesystem.
pub fn normalize_lexically(path: &Path) -> PathBuf {
    let mut out = PathBuf::new();
    for component in path.components() {
        match component {
            Component::CurDir => {}
            Component::ParentDir => {
                out.pop();
            }
            other => out.push(other),
        }
    }
    out
}

/// Serialize JSON with 2-space indentation and a trailing newline.
pub fn pretty_json(value: &Value) -> String {
    let mut text = serde_json::to_string_pretty(value).expect("JSON values always serialize");
    text.push('\n');
    text
}

/// Compact JSON with `", "` and `": "` separators (human-readable inline form).
pub fn inline_json(value: &Value) -> String {
    match value {
        Value::Object(map) => {
            let parts: Vec<String> = map
                .iter()
                .map(|(key, item)| format!("{}: {}", Value::String(key.clone()), inline_json(item)))
                .collect();
            format!("{{{}}}", parts.join(", "))
        }
        Value::Array(items) => {
            let parts: Vec<String> = items.iter().map(inline_json).collect();
            format!("[{}]", parts.join(", "))
        }
        other => other.to_string(),
    }
}

/// Write `contents` to `path`, exiting with a clear message on failure.
pub fn write_or_die(path: &Path, contents: &str) {
    if let Err(error) = std::fs::write(path, contents) {
        die(format!("Could not write {}: {error}", path.display()));
    }
}
