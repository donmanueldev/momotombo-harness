//! Audit rendered HTML and site-level discovery files (robots.txt, sitemaps, llms.txt).

use agent_tools::{die, normalize_lexically, now_iso, pretty_json, resolve, write_or_die};
use clap::{Parser, ValueEnum};
use scraper::Html;
use serde_json::{json, Map, Value};
use std::collections::{BTreeSet, HashMap};
use std::io::Read;
use std::path::{Path, PathBuf};
use std::time::Duration;
use ureq::ResponseExt;
use url::Url;

const USER_AGENT: &str = "agent-config-seo-audit/1.0";
const SEVERITIES: [&str; 3] = ["info", "warning", "error"];

#[derive(Parser)]
#[command(about = "Audit rendered HTML and site-level discovery files.")]
struct Args {
    /// Public or locally served URL; repeat for multiple pages
    #[arg(long = "url", required = true)]
    urls: Vec<String>,
    /// Generated HTML paired by order with --url; repeat for multiple pages
    #[arg(long = "html-file")]
    html_files: Vec<PathBuf>,
    /// Validate local robots.txt, sitemap.xml, and optional llms.txt
    #[arg(long)]
    site_root: Option<PathBuf>,
    /// Directory for report.json and report.md
    #[arg(long, default_value = "seo-audit")]
    output_dir: PathBuf,
    #[arg(long, default_value_t = 15.0)]
    timeout: f64,
    #[arg(long, default_value_t = 5_000_000)]
    max_bytes: u64,
    #[arg(long)]
    skip_site_files: bool,
    #[arg(long, value_enum, default_value_t = FailOn::Error)]
    fail_on: FailOn,
}

#[derive(Clone, Copy, ValueEnum)]
enum FailOn {
    Error,
    Warning,
    Never,
}

fn rank(severity: &str) -> usize {
    SEVERITIES.iter().position(|item| *item == severity).unwrap_or(0)
}

fn clean_text(value: &str) -> String {
    value.split_whitespace().collect::<Vec<_>>().join(" ")
}

// ---------------------------------------------------------------- URL helpers

fn join(base: &str, href: &str) -> String {
    Url::parse(base).and_then(|base| base.join(href)).map(String::from).unwrap_or_else(|_| href.to_string())
}

fn normalize(url: &str) -> String {
    Url::parse(url).map(String::from).unwrap_or_else(|_| url.to_string())
}

fn is_http(url: &str) -> bool {
    Url::parse(url).is_ok_and(|url| matches!(url.scheme(), "http" | "https"))
}

fn netloc(url: &str) -> String {
    match Url::parse(url) {
        Ok(url) => match (url.host_str(), url.port()) {
            (Some(host), Some(port)) => format!("{host}:{port}"),
            (Some(host), None) => host.to_string(),
            _ => String::new(),
        },
        Err(_) => String::new(),
    }
}

fn origin_of(url: &str) -> String {
    let scheme = Url::parse(url).map(|url| url.scheme().to_string()).unwrap_or_default();
    format!("{scheme}://{}", netloc(url))
}

// ---------------------------------------------------------------- Fetching

struct Response {
    ok: bool,
    status: Option<u16>,
    url: String,
    content_type: String,
    headers: HashMap<String, String>,
    body: String,
    truncated: bool,
    error: Option<String>,
}

fn fetch(url: &str, timeout: f64, max_bytes: u64) -> Response {
    let agent: ureq::Agent = ureq::Agent::config_builder()
        .timeout_global(Some(Duration::from_secs_f64(timeout)))
        .http_status_as_error(false)
        .user_agent(USER_AGENT)
        .build()
        .into();
    let failure = |error: String| Response {
        ok: false,
        status: None,
        url: url.to_string(),
        content_type: String::new(),
        headers: HashMap::new(),
        body: String::new(),
        truncated: false,
        error: Some(error),
    };
    let mut response = match agent.get(url).header("Accept", "*/*").call() {
        Ok(response) => response,
        Err(error) => return failure(error.to_string()),
    };
    let status = response.status();
    let final_url = response.get_uri().to_string();
    let headers: HashMap<String, String> = response
        .headers()
        .iter()
        .map(|(name, value)| (name.as_str().to_lowercase(), value.to_str().unwrap_or_default().to_string()))
        .collect();
    let mut bytes = Vec::new();
    if let Err(error) = response.body_mut().as_reader().take(max_bytes + 1).read_to_end(&mut bytes) {
        return failure(error.to_string());
    }
    let ok = status.as_u16() < 400;
    let truncated = ok && bytes.len() as u64 > max_bytes;
    bytes.truncate(max_bytes as usize);
    Response {
        ok,
        status: Some(status.as_u16()),
        url: final_url,
        content_type: headers.get("content-type").cloned().unwrap_or_default(),
        headers,
        body: String::from_utf8_lossy(&bytes).into_owned(),
        truncated,
        error: (!ok).then(|| format!("HTTP Error {}: {}", status.as_u16(), status.canonical_reason().unwrap_or(""))),
    }
}

fn local_response(url: &str, body: String) -> Response {
    Response {
        ok: true,
        status: Some(200),
        url: url.to_string(),
        content_type: "text/html; charset=utf-8".into(),
        headers: HashMap::new(),
        body,
        truncated: false,
        error: None,
    }
}

// ---------------------------------------------------------------- HTML parsing

type Attrs = HashMap<String, String>;

#[derive(Default)]
struct Document {
    title: String,
    html_lang: String,
    headings: Vec<(u8, String)>,
    metas: Vec<Attrs>,
    links: Vec<Attrs>,
    images: Vec<Attrs>,
    canonicals: Vec<String>,
    json_ld_raw: Vec<String>,
}

fn parse_document(body: &str) -> Document {
    let html = Html::parse_document(body);
    let mut doc = Document::default();
    let mut title_parts = Vec::new();
    for node in html.root_element().descendants() {
        let Some(element) = scraper::ElementRef::wrap(node) else { continue };
        let attrs: Attrs = element.value().attrs().map(|(key, value)| (key.to_lowercase(), value.to_string())).collect();
        let text = || element.text().collect::<String>();
        match element.value().name() {
            "html" => doc.html_lang = attrs.get("lang").cloned().unwrap_or_default(),
            "title" => title_parts.push(text()),
            "meta" => doc.metas.push(attrs),
            "link" => {
                let is_canonical = attrs.get("rel").is_some_and(|rel| rel.split_whitespace().any(|part| part.eq_ignore_ascii_case("canonical")));
                if let Some(href) = attrs.get("href").filter(|href| is_canonical && !href.is_empty()) {
                    doc.canonicals.push(href.clone());
                }
                doc.links.push(attrs);
            }
            "a" => doc.links.push(attrs),
            "img" => doc.images.push(attrs),
            name @ ("h1" | "h2" | "h3" | "h4" | "h5" | "h6") => {
                doc.headings.push((name.as_bytes()[1] - b'0', clean_text(&text())));
            }
            "script" if attrs.get("type").is_some_and(|kind| kind.eq_ignore_ascii_case("application/ld+json")) => {
                doc.json_ld_raw.push(text().trim().to_string());
            }
            _ => {}
        }
    }
    doc.title = clean_text(&title_parts.concat());
    doc
}

fn meta_values(doc: &Document, key: &str, value: &str) -> Vec<String> {
    doc.metas
        .iter()
        .filter(|meta| meta.get(key).is_some_and(|found| found.to_lowercase() == value))
        .map(|meta| meta.get("content").cloned().unwrap_or_default())
        .collect()
}

fn json_ld_types(value: &Value, found: &mut Vec<String>) {
    match value {
        Value::Object(map) => {
            match map.get("@type") {
                Some(Value::String(kind)) => found.push(kind.clone()),
                Some(Value::Array(kinds)) => found.extend(kinds.iter().filter_map(Value::as_str).map(String::from)),
                _ => {}
            }
            map.values().for_each(|nested| json_ld_types(nested, found));
        }
        Value::Array(items) => items.iter().for_each(|nested| json_ld_types(nested, found)),
        _ => {}
    }
}

// ---------------------------------------------------------------- Findings

fn finding(code: &str, severity: &str, message: &str, evidence: Value) -> Value {
    let mut item = json!({"code": code, "severity": severity, "message": message});
    let empty = match &evidence {
        Value::Null => true,
        Value::String(text) => text.is_empty(),
        Value::Array(items) => items.is_empty(),
        Value::Object(map) => map.is_empty(),
        _ => false,
    };
    if !empty {
        item["evidence"] = evidence;
    }
    item
}

fn audit_page(requested_url: &str, response: &Response) -> Value {
    let mut findings = Vec::new();
    let status = response.status;
    if !response.ok {
        findings.push(finding("http.fetch", "error", "Page could not be fetched successfully.", json!(response.error)));
    } else if !status.is_some_and(|code| (200..300).contains(&code)) {
        findings.push(finding("http.status", "error", &format!("Expected a successful response, received {}.", json!(status)), Value::Null));
    }
    if response.truncated {
        findings.push(finding("http.truncated", "warning", "HTML exceeded the configured byte limit; results may be incomplete.", Value::Null));
    }
    let content_type = &response.content_type;
    if !content_type.is_empty() && !content_type.to_lowercase().contains("html") {
        findings.push(finding("http.content-type", "error", "Response is not identified as HTML.", json!(content_type)));
    }

    let doc = parse_document(&response.body);
    let final_url = if response.url.is_empty() { requested_url } else { response.url.as_str() };
    let final_normalized = normalize(final_url);
    let descriptions = meta_values(&doc, "name", "description");
    let robots: Vec<String> = meta_values(&doc, "name", "robots").iter().map(|value| value.to_lowercase()).collect();
    let x_robots: Vec<String> = response.headers.get("x-robots-tag").filter(|v| !v.is_empty()).map(|v| v.to_lowercase()).into_iter().collect();
    let og_names = ["og:title", "og:type", "og:image", "og:url", "og:description", "og:image:alt"];
    let og: Map<String, Value> = og_names.iter().map(|name| (name.to_string(), json!(meta_values(&doc, "property", name)))).collect();
    let og_list = |name: &str| -> Vec<String> {
        og[name].as_array().into_iter().flatten().filter_map(Value::as_str).map(String::from).collect()
    };

    if doc.title.is_empty() {
        findings.push(finding("metadata.title.missing", "error", "Document has no non-empty title.", Value::Null));
    }
    if descriptions.len() != 1 || clean_text(&descriptions[0]).is_empty() {
        findings.push(finding("metadata.description", "error", "Expected one non-empty meta description.", json!(descriptions)));
    }
    let canonical = (doc.canonicals.len() == 1).then(|| join(final_url, &doc.canonicals[0]));
    match &canonical {
        None => findings.push(finding("canonical.count", "error", "Expected exactly one canonical link.", json!(doc.canonicals))),
        Some(resolved) => {
            if !is_http(&doc.canonicals[0]) {
                findings.push(finding("canonical.absolute-url", "warning", "Canonical should use an absolute HTTP(S) URL.", json!(doc.canonicals[0])));
            }
            if *resolved != final_normalized {
                findings.push(finding(
                    "canonical.target",
                    "warning",
                    "Canonical differs from the fetched final URL; verify that consolidation is intentional.",
                    json!({"canonical": resolved, "finalUrl": final_url}),
                ));
            }
        }
    }
    let noindex = robots.iter().chain(&x_robots).flat_map(|value| value.split(',')).any(|token| token.trim() == "noindex");
    if noindex {
        findings.push(finding("robots.noindex", "error", "The audited public URL declares noindex.", json!({"meta": robots, "headers": x_robots})));
    }
    if doc.html_lang.is_empty() {
        findings.push(finding("html.lang", "warning", "The html element has no language declaration.", Value::Null));
    }

    let headings: Vec<Value> = doc.headings.iter().map(|(level, text)| json!({"level": level, "text": text})).collect();
    let h1_count = doc.headings.iter().filter(|(level, _)| *level == 1).count();
    if h1_count == 0 {
        findings.push(finding("headings.main-topic", "warning", "No h1 identifies the page's main topic.", Value::Null));
    } else if h1_count > 1 {
        findings.push(finding("headings.multiple-h1", "info", "Multiple h1 elements found; confirm that the document remains semantically clear.", json!(h1_count)));
    }
    let mut previous = 0;
    for (heading, (level, _)) in headings.iter().zip(&doc.headings) {
        if previous != 0 && *level > previous + 1 {
            findings.push(finding("headings.skipped-level", "warning", "Heading hierarchy skips a level.", heading.clone()));
        }
        previous = *level;
    }

    for required in ["og:title", "og:type", "og:image", "og:url"] {
        let values = og_list(required);
        if values.len() != 1 || clean_text(&values[0]).is_empty() {
            let code = format!("open-graph.{}", required[3..].replace(':', "-"));
            findings.push(finding(&code, "error", &format!("Expected one non-empty {required} property."), json!(values)));
        }
    }
    for property in ["og:image", "og:url"] {
        if let Some(first) = og_list(property).first().filter(|value| !is_http(value)) {
            findings.push(finding("open-graph.absolute-url", "error", &format!("{property} must use an absolute HTTP(S) URL."), json!(first)));
        }
    }
    if let Some(og_url) = og_list("og:url").first() {
        if join(final_url, og_url) != final_normalized {
            findings.push(finding("open-graph.url-mismatch", "warning", "og:url differs from the fetched final URL.", json!({"og:url": og_url, "finalUrl": final_url})));
        }
    }
    if !og_list("og:image").is_empty() && og_list("og:image:alt").is_empty() {
        findings.push(finding("open-graph.image-alt", "warning", "Open Graph image has no og:image:alt.", Value::Null));
    }

    let src = |image: &Attrs| image.get("src").cloned().unwrap_or_default();
    let missing_alt: Vec<String> = doc.images.iter().filter(|image| !image.contains_key("alt")).map(src).collect();
    let missing_dimensions: Vec<String> = doc
        .images
        .iter()
        .filter(|image| image.get("width").is_none_or(|v| v.is_empty()) || image.get("height").is_none_or(|v| v.is_empty()))
        .map(src)
        .collect();
    if !missing_alt.is_empty() {
        findings.push(finding("images.alt", "warning", "Images without an alt attribute found.", json!(missing_alt)));
    }
    if !missing_dimensions.is_empty() {
        findings.push(finding("images.dimensions", "warning", "Images without explicit width and height found.", json!(missing_dimensions)));
    }

    let mut parsed_json_ld = Vec::new();
    let mut invalid_json_ld = Vec::new();
    for raw in &doc.json_ld_raw {
        match serde_json::from_str::<Value>(raw) {
            Ok(value) => parsed_json_ld.push(value),
            Err(error) => invalid_json_ld.push(error.to_string()),
        }
    }
    if !invalid_json_ld.is_empty() {
        findings.push(finding("json-ld.invalid", "error", "Invalid JSON-LD block found.", json!(invalid_json_ld)));
    }
    if doc.json_ld_raw.is_empty() {
        findings.push(finding("json-ld.absent", "info", "No JSON-LD found; add it only when an applicable type matches visible content.", Value::Null));
    }
    let mut types = Vec::new();
    json_ld_types(&Value::Array(parsed_json_ld), &mut types);
    let types: BTreeSet<String> = types.into_iter().collect();

    let origin_netloc = netloc(final_url);
    let internal_links = doc
        .links
        .iter()
        .filter_map(|link| link.get("href"))
        .filter(|href| !href.is_empty() && !["#", "mailto:", "tel:", "javascript:"].iter().any(|prefix| href.starts_with(prefix)))
        .filter(|href| netloc(&join(final_url, href)) == origin_netloc)
        .count();
    if internal_links == 0 {
        findings.push(finding("links.internal", "warning", "No crawlable internal links were found.", Value::Null));
    }

    json!({
        "requestedUrl": requested_url,
        "finalUrl": final_url,
        "status": status,
        "contentType": content_type,
        "title": doc.title,
        "description": descriptions.first(),
        "canonical": canonical,
        "robots": {"meta": robots, "headers": x_robots},
        "openGraph": og,
        "headings": headings,
        "images": {"count": doc.images.len(), "missingAlt": missing_alt, "missingDimensions": missing_dimensions},
        "internalLinkCount": internal_links,
        "jsonLd": {"blocks": doc.json_ld_raw.len(), "types": types, "invalid": invalid_json_ld},
        "findings": findings,
    })
}

// ---------------------------------------------------------------- Site files

fn xml_error(text: &str) -> Option<String> {
    let options = roxmltree::ParsingOptions { allow_dtd: true, ..Default::default() };
    roxmltree::Document::parse_with_options(text, options).err().map(|error| error.to_string())
}

fn declared_sitemaps(robots_body: &str) -> Vec<String> {
    robots_body
        .lines()
        .filter(|line| line.to_lowercase().starts_with("sitemap:"))
        .filter_map(|line| line.split_once(':').map(|(_, value)| value.trim().to_string()))
        .collect()
}

fn audit_site_files(urls: &[String], timeout: f64, max_bytes: u64) -> Vec<Value> {
    let origins: BTreeSet<String> = urls.iter().map(|url| origin_of(url)).collect();
    origins
        .into_iter()
        .map(|origin| {
            let robots_url = join(&origin, "/robots.txt");
            let robots = fetch(&robots_url, timeout, max_bytes);
            let mut sitemap_urls = Vec::new();
            let mut external = Vec::new();
            if robots.ok {
                for declared in declared_sitemaps(&robots.body) {
                    let sitemap_url = join(&origin, &declared);
                    if netloc(&sitemap_url) == netloc(&origin) {
                        sitemap_urls.push(sitemap_url);
                    } else {
                        external.push(sitemap_url);
                    }
                }
            }
            if sitemap_urls.is_empty() {
                sitemap_urls.push(join(&origin, "/sitemap.xml"));
            }
            let sitemaps: Vec<Value> = sitemap_urls
                .iter()
                .map(|sitemap_url| {
                    let response = fetch(sitemap_url, timeout, max_bytes);
                    let parse_error = response.ok.then(|| xml_error(&response.body)).flatten();
                    let valid = response.ok && parse_error.is_none();
                    json!({"url": sitemap_url, "status": response.status, "validXml": valid, "error": parse_error.or(response.error)})
                })
                .collect();
            let llms_url = join(&origin, "/llms.txt");
            let llms = fetch(&llms_url, timeout, max_bytes);
            json!({
                "origin": origin,
                "robots": {"url": robots_url, "status": robots.status, "body": robots.ok.then_some(&robots.body), "error": robots.error},
                "sitemaps": sitemaps,
                "externalSitemaps": external,
                "llmsTxt": {"url": llms_url, "status": llms.status, "present": llms.ok, "required": false},
            })
        })
        .collect()
}

/// Path and netloc of a sitemap declaration that may be absolute or relative.
fn split_declared(url: &str) -> (String, String) {
    match Url::parse(url) {
        Ok(parsed) => (netloc(url), parsed.path().to_string()),
        Err(_) => (String::new(), url.split(['?', '#']).next().unwrap_or_default().to_string()),
    }
}

fn audit_local_site_files(root: &Path, public_url: &str) -> Vec<Value> {
    let public_netloc = netloc(public_url);
    let origin = origin_of(public_url);
    let robots_path = root.join("robots.txt");
    let robots_exists = robots_path.is_file();
    let robots_body = if robots_exists {
        String::from_utf8_lossy(&std::fs::read(&robots_path).unwrap_or_default()).into_owned()
    } else {
        String::new()
    };
    let declared = declared_sitemaps(&robots_body);
    let mut sitemap_paths: Vec<PathBuf> = Vec::new();
    for sitemap_url in &declared {
        let (host, path) = split_declared(sitemap_url);
        if !host.is_empty() && host != public_netloc {
            continue;
        }
        let candidate = normalize_lexically(&root.join(path.trim_start_matches('/')));
        if candidate != root && !candidate.starts_with(root) {
            continue;
        }
        if !sitemap_paths.contains(&candidate) {
            sitemap_paths.push(candidate);
        }
    }
    if sitemap_paths.is_empty() {
        sitemap_paths.push(root.join("sitemap.xml"));
    }
    let sitemaps: Vec<Value> = sitemap_paths
        .iter()
        .map(|path| {
            let exists = path.is_file();
            let error = if exists { xml_error(&String::from_utf8_lossy(&std::fs::read(path).unwrap_or_default())) } else { None };
            let relative = path.strip_prefix(root).unwrap_or(path).display().to_string();
            json!({
                "url": join(&origin, &format!("/{relative}")),
                "status": if exists { 200 } else { 404 },
                "validXml": exists && error.is_none(),
                "error": error,
            })
        })
        .collect();
    let external: Vec<&String> = declared
        .iter()
        .filter(|url| {
            let host = split_declared(url).0;
            !host.is_empty() && host != public_netloc
        })
        .collect();
    let llms_exists = root.join("llms.txt").is_file();
    vec![json!({
        "origin": origin,
        "robots": {
            "url": join(&origin, "/robots.txt"),
            "status": if robots_exists { 200 } else { 404 },
            "body": robots_exists.then_some(robots_body),
            "error": null,
        },
        "sitemaps": sitemaps,
        "externalSitemaps": external,
        "llmsTxt": {"url": join(&origin, "/llms.txt"), "status": if llms_exists { 200 } else { 404 }, "present": llms_exists, "required": false},
    })]
}

fn generic_robots_blocks_all(body: &str) -> bool {
    let mut agents: Vec<String> = Vec::new();
    let mut saw_directive = false;
    for raw_line in body.lines() {
        let line = raw_line.split('#').next().unwrap_or_default().trim();
        let Some((key, value)) = line.split_once(':') else { continue };
        let (key, value) = (key.trim().to_lowercase(), value.trim());
        if key == "user-agent" {
            if saw_directive {
                agents.clear();
                saw_directive = false;
            }
            agents.push(value.to_lowercase());
            continue;
        }
        saw_directive = true;
        if key == "disallow" && agents.iter().any(|agent| agent == "*") && value == "/" {
            return true;
        }
    }
    false
}

fn attach_site_file_findings(site: &mut Value) {
    let mut findings = Vec::new();
    let robots = &site["robots"];
    if robots["status"] == json!(404) {
        findings.push(finding("robots-txt.absent", "info", "robots.txt is absent; this is valid when default crawl behavior is intended.", Value::Null));
    } else if robots["status"] != json!(200) {
        findings.push(finding("robots-txt.fetch", "warning", "robots.txt did not return HTTP 200.", robots["status"].clone()));
    } else if generic_robots_blocks_all(robots["body"].as_str().unwrap_or_default()) {
        findings.push(finding("robots-txt.blocks-all", "error", "The generic robots.txt group disallows the entire site.", Value::Null));
    }
    for sitemap in site["sitemaps"].as_array().into_iter().flatten() {
        if sitemap["status"] == json!(404) {
            findings.push(finding("sitemap.absent", "info", "No sitemap was found at the discovered location.", sitemap["url"].clone()));
        } else if sitemap["status"] != json!(200) {
            findings.push(finding("sitemap.fetch", "warning", "Sitemap did not return HTTP 200.", json!({"url": sitemap["url"], "status": sitemap["status"]})));
        } else if sitemap["validXml"] != json!(true) {
            findings.push(finding("sitemap.invalid-xml", "error", "Sitemap is not valid XML.", json!({"url": sitemap["url"], "error": sitemap["error"]})));
        }
    }
    if site["externalSitemaps"].as_array().is_some_and(|items| !items.is_empty()) {
        findings.push(finding("sitemap.external", "warning", "External sitemap declarations were recorded but not fetched automatically.", site["externalSitemaps"].clone()));
    }
    if site["llmsTxt"]["present"] != json!(true) {
        findings.push(finding("llms-txt.absent", "info", "llms.txt is absent; it is experimental and not required.", Value::Null));
    }
    site["findings"] = json!(findings);
}

fn attach_duplicate_metadata_findings(pages: &mut [Value]) {
    for (field, code, label) in [
        ("title", "metadata.title.duplicate", "title"),
        ("description", "metadata.description.duplicate", "meta description"),
        ("canonical", "canonical.duplicate", "canonical URL"),
    ] {
        let mut groups: Vec<(String, Vec<usize>)> = Vec::new();
        for (index, page) in pages.iter().enumerate() {
            let value = clean_text(page[field].as_str().unwrap_or_default());
            if value.is_empty() {
                continue;
            }
            match groups.iter_mut().find(|(existing, _)| *existing == value) {
                Some((_, indices)) => indices.push(index),
                None => groups.push((value, vec![index])),
            }
        }
        for (value, indices) in groups.into_iter().filter(|(_, indices)| indices.len() > 1) {
            let urls: Vec<Value> = indices.iter().map(|&index| pages[index]["requestedUrl"].clone()).collect();
            for &index in &indices {
                let item = finding(code, "warning", &format!("The same {label} appears on multiple audited URLs."), json!({"value": value, "urls": urls}));
                if let Some(findings) = pages[index]["findings"].as_array_mut() {
                    findings.push(item);
                }
            }
        }
    }
}

// ---------------------------------------------------------------- Report

/// Render a JSON scalar the way the report has always shown it (None/True/False).
fn display(value: &Value) -> String {
    match value {
        Value::Null => "None".into(),
        Value::Bool(true) => "True".into(),
        Value::Bool(false) => "False".into(),
        Value::String(text) => text.clone(),
        other => other.to_string(),
    }
}

fn non_empty_or<'a>(value: &'a Value, fallback: &'a str) -> &'a str {
    value.as_str().filter(|text| !text.is_empty()).unwrap_or(fallback)
}

fn markdown_report(report: &Value) -> String {
    let summary = &report["summary"];
    let pages = report["pages"].as_array().cloned().unwrap_or_default();
    let mut lines = vec![
        "# Website SEO Finalization Audit".to_string(),
        String::new(),
        format!("Generated: `{}`", display(&report["generatedAt"])),
        String::new(),
        "## Summary".into(),
        String::new(),
        format!("- URLs: {}", pages.len()),
        format!("- Errors: {}", summary["error"]),
        format!("- Warnings: {}", summary["warning"]),
        format!("- Informational: {}", summary["info"]),
        String::new(),
    ];
    for page in &pages {
        lines.extend([
            format!("## {}", display(&page["requestedUrl"])),
            String::new(),
            format!("- Status: `{}`", display(&page["status"])),
            format!("- Final URL: `{}`", display(&page["finalUrl"])),
            format!("- Title: {}", non_empty_or(&page["title"], "_missing_")),
            format!("- Canonical: `{}`", non_empty_or(&page["canonical"], "missing")),
            String::new(),
            "| Severity | Code | Finding |".into(),
            "| --- | --- | --- |".into(),
        ]);
        let findings = page["findings"].as_array().cloned().unwrap_or_default();
        if findings.is_empty() {
            lines.push("| - | - | No automated findings |".into());
        }
        for item in findings {
            let message = display(&item["message"]).replace('|', "\\|");
            lines.push(format!("| {} | `{}` | {message} |", display(&item["severity"]), display(&item["code"])));
        }
        lines.push(String::new());
    }
    lines.extend(["## Site files".into(), String::new()]);
    for site in report["siteFiles"].as_array().into_iter().flatten() {
        lines.push(format!(
            "- `{}`: robots `{}`, llms.txt `{}` (optional)",
            display(&site["origin"]),
            display(&site["robots"]["status"]),
            display(&site["llmsTxt"]["status"])
        ));
        for sitemap in site["sitemaps"].as_array().into_iter().flatten() {
            lines.push(format!(
                "  - sitemap `{}`: status `{}`, valid XML `{}`",
                display(&sitemap["url"]),
                display(&sitemap["status"]),
                display(&sitemap["validXml"])
            ));
        }
        for item in site["findings"].as_array().into_iter().flatten() {
            lines.push(format!("  - {}: `{}` — {}", display(&item["severity"]), display(&item["code"]), display(&item["message"])));
        }
    }
    lines.extend([
        String::new(),
        "> Automated findings require contextual review. A clean report does not prove indexing, rankings, rich results, or LLM citations.".into(),
        String::new(),
    ]);
    lines.join("\n")
}

fn main() {
    let args = Args::parse();
    if !args.html_files.is_empty() && args.urls.len() != args.html_files.len() {
        die("Provide one --html-file for each --url");
    }
    if args.site_root.is_some() && args.html_files.is_empty() {
        die("--site-root is supported with --html-file");
    }
    for url in &args.urls {
        if !is_http(url) || netloc(url).is_empty() {
            die(format!("Invalid HTTP(S) URL: {url}"));
        }
    }
    if args.site_root.is_some() && args.urls.iter().map(|url| netloc(url)).collect::<BTreeSet<_>>().len() != 1 {
        die("--site-root requires all URLs to share one origin");
    }

    let mut pages: Vec<Value> = if args.html_files.is_empty() {
        args.urls.iter().map(|url| audit_page(url, &fetch(url, args.timeout, args.max_bytes))).collect()
    } else {
        args.urls
            .iter()
            .zip(&args.html_files)
            .map(|(url, file)| {
                let bytes = std::fs::read(resolve(file)).unwrap_or_else(|error| die(format!("Could not read {}: {error}", file.display())));
                audit_page(url, &local_response(url, String::from_utf8_lossy(&bytes).into_owned()))
            })
            .collect()
    };
    let mut site_files = if args.skip_site_files {
        Vec::new()
    } else if let Some(root) = &args.site_root {
        audit_local_site_files(&resolve(root), &args.urls[0])
    } else {
        audit_site_files(&args.urls, args.timeout, args.max_bytes)
    };
    site_files.iter_mut().for_each(attach_site_file_findings);
    attach_duplicate_metadata_findings(&mut pages);

    let all_findings: Vec<Value> = pages
        .iter()
        .chain(&site_files)
        .flat_map(|item| item["findings"].as_array().cloned().unwrap_or_default())
        .collect();
    let mut summary = Map::new();
    for severity in SEVERITIES {
        let count = all_findings.iter().filter(|item| item["severity"] == json!(severity)).count();
        summary.insert(severity.into(), json!(count));
    }

    let report = json!({
        "schemaVersion": 1,
        "generatedAt": now_iso(),
        "pages": pages,
        "siteFiles": site_files,
        "summary": summary,
    });
    if let Err(error) = std::fs::create_dir_all(&args.output_dir) {
        die(format!("Could not create {}: {error}", args.output_dir.display()));
    }
    let output_dir = resolve(&args.output_dir);
    let json_path = output_dir.join("report.json");
    let markdown_path = output_dir.join("report.md");
    write_or_die(&json_path, &pretty_json(&report));
    write_or_die(&markdown_path, &markdown_report(&report));
    println!("Wrote {}", json_path.display());
    println!("Wrote {}", markdown_path.display());

    let threshold = match args.fail_on {
        FailOn::Never => std::process::exit(0),
        FailOn::Error => rank("error"),
        FailOn::Warning => rank("warning"),
    };
    let failed = all_findings.iter().any(|item| rank(item["severity"].as_str().unwrap_or("info")) >= threshold);
    std::process::exit(if failed { 2 } else { 0 });
}
