//! Detect a React, Next.js, Astro, or static HTML audit adapter.

use agent_tools::resolve;
use clap::{Parser, ValueEnum};
use serde_json::{json, Map, Value};
use std::path::{Path, PathBuf};

#[derive(Parser)]
#[command(about = "Detect a React, Next.js, Astro, or static HTML audit adapter.")]
struct Args {
    #[arg(long, default_value = ".")]
    root: PathBuf,
    #[arg(long, default_value_t = 4173)]
    port: u16,
    #[arg(long, value_enum, default_value_t = Format::Json)]
    format: Format,
}

#[derive(Clone, Copy, ValueEnum)]
enum Format {
    Json,
    Markdown,
}

fn package_manager(root: &Path) -> (&'static str, Vec<&'static str>) {
    if root.join("bun.lock").exists() || root.join("bun.lockb").exists() {
        ("bun", vec!["bun", "run"])
    } else if root.join("pnpm-lock.yaml").exists() {
        ("pnpm", vec!["pnpm", "run"])
    } else if root.join("yarn.lock").exists() {
        ("yarn", vec!["yarn"])
    } else {
        ("npm", vec!["npm", "run"])
    }
}

fn command(prefix: &[&str], script: &str, extra: &[String]) -> Value {
    let mut parts: Vec<String> = prefix.iter().map(|part| part.to_string()).collect();
    parts.push(script.to_string());
    parts.extend(extra.iter().cloned());
    json!(parts)
}

/// Local static server fallback. PHP's built-in server needs no downloads.
fn static_server(port: u16, directory: &Path) -> Value {
    json!(["php", "-S", format!("127.0.0.1:{port}"), "-t", directory.display().to_string()])
}

fn detect(root: &Path, port: u16) -> Result<Value, String> {
    let url = format!("http://127.0.0.1:{port}/");
    let package_file = root.join("package.json");
    if !package_file.exists() {
        let notes: Vec<&str> = if root.join("index.html").exists() {
            vec![]
        } else {
            vec!["No index.html found at the selected root."]
        };
        return Ok(json!({
            "framework": "static-html",
            "packageManager": null,
            "build": null,
            "serve": static_server(port, root),
            "outputDirectory": root.display().to_string(),
            "url": url,
            "notes": notes,
        }));
    }

    let text = std::fs::read_to_string(&package_file).map_err(|error| error.to_string())?;
    let data: Value = serde_json::from_str(&text).map_err(|error| error.to_string())?;
    let scripts = data.get("scripts").and_then(Value::as_object).cloned().unwrap_or_default();
    let mut dependencies = Map::new();
    for key in ["dependencies", "devDependencies"] {
        if let Some(group) = data.get(key).and_then(Value::as_object) {
            for (name, version) in group {
                dependencies.insert(name.clone(), version.clone());
            }
        }
    }
    let has = |name: &str| dependencies.contains_key(name);
    let (manager, run) = package_manager(root);
    let port_text = port.to_string();
    let host_port: Vec<String> = ["--", "--host", "127.0.0.1", "--port", port_text.as_str()]
        .iter()
        .map(|part| part.to_string())
        .collect();
    let mut notes: Vec<String> = Vec::new();
    let build = if scripts.contains_key("build") { command(&run, "build", &[]) } else { Value::Null };

    let (framework, output_directory, serve) = if has("next") {
        let serve = if scripts.contains_key("start") {
            command(&run, "start", &["--".into(), "-p".into(), port_text.clone()])
        } else {
            notes.push("No start script found; add a project-owned `next start` script. The adapter will not download tooling implicitly.".into());
            Value::Null
        };
        ("nextjs", json!(".next"), serve)
    } else if has("astro") {
        let serve = if scripts.contains_key("preview") {
            command(&run, "preview", &host_port)
        } else {
            notes.push("No preview script found; add `astro preview` or serve dist for a static output.".into());
            Value::Null
        };
        ("astro", json!("dist"), serve)
    } else if has("react") && has("vite") {
        let serve = if scripts.contains_key("preview") {
            command(&run, "preview", &host_port)
        } else {
            notes.push("No preview script found; add `vite preview` for production-build audits.".into());
            Value::Null
        };
        ("react-vite", json!("dist"), serve)
    } else if has("react") && has("react-scripts") {
        notes.push("The static server does not emulate production rewrites or headers.".into());
        ("react-cra", json!("build"), static_server(port, &root.join("build")))
    } else {
        notes.push("Framework was not recognized; verify build output, production server, and port manually.".into());
        let serve = if scripts.contains_key("preview") {
            command(&run, "preview", &[])
        } else if scripts.contains_key("start") {
            command(&run, "start", &[])
        } else {
            Value::Null
        };
        (if has("react") { "react-custom" } else { "node-web" }, Value::Null, serve)
    };

    if build.is_null() {
        notes.push("No build script found; do not substitute a development server for production evidence.".into());
    }
    Ok(json!({
        "framework": framework,
        "packageManager": manager,
        "build": build,
        "serve": serve,
        "outputDirectory": output_directory,
        "url": url,
        "scripts": scripts,
        "notes": notes,
    }))
}

fn show(value: &Value, fallback: &str) -> String {
    match value {
        Value::Array(parts) => parts
            .iter()
            .map(|part| {
                let text = part.as_str().unwrap_or_default();
                if text.contains(' ') { Value::String(text.into()).to_string() } else { text.to_string() }
            })
            .collect::<Vec<_>>()
            .join(" "),
        Value::String(text) if !text.is_empty() => text.clone(),
        _ => fallback.to_string(),
    }
}

fn markdown(result: &Value) -> String {
    let mut lines = vec![
        "# Web Audit Adapter".to_string(),
        String::new(),
        format!("- Framework: `{}`", show(&result["framework"], "manual")),
        format!("- Package manager: `{}`", show(&result["packageManager"], "none")),
        format!("- Build: `{}`", show(&result["build"], "manual")),
        format!("- Serve: `{}`", show(&result["serve"], "manual")),
        format!("- Expected URL: `{}`", show(&result["url"], "manual")),
    ];
    if let Some(notes) = result["notes"].as_array().filter(|notes| !notes.is_empty()) {
        lines.extend([String::new(), "## Verify".into(), String::new()]);
        lines.extend(notes.iter().map(|note| format!("- {}", note.as_str().unwrap_or_default())));
    }
    lines.join("\n") + "\n"
}

fn main() {
    let args = Args::parse();
    let root = resolve(&args.root);
    match detect(&root, args.port) {
        Ok(result) => match args.format {
            Format::Json => println!("{}", serde_json::to_string_pretty(&result).unwrap()),
            Format::Markdown => print!("{}", markdown(&result)),
        },
        Err(error) => {
            eprintln!("Could not inspect project: {error}");
            std::process::exit(1);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn show_quotes_parts_with_spaces_and_falls_back() {
        assert_eq!(show(&json!(["npm", "run", "a b"]), "manual"), r#"npm run "a b""#);
        assert_eq!(show(&Value::Null, "none"), "none");
    }

    #[test]
    fn static_server_uses_php() {
        assert_eq!(static_server(8080, Path::new("/site")), json!(["php", "-S", "127.0.0.1:8080", "-t", "/site"]));
    }
}
