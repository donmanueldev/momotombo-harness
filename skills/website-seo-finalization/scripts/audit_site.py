#!/usr/bin/env python3
"""Audit rendered HTML and site-level discovery files without third-party packages."""

from __future__ import annotations

import argparse
import html
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import sys
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.parse import urljoin, urlparse
from urllib.request import Request, urlopen
import xml.etree.ElementTree as ET


USER_AGENT = "agent-config-seo-audit/1.0"
SEVERITY_RANK = {"info": 0, "warning": 1, "error": 2}


def clean_text(value: str) -> str:
    return re.sub(r"\s+", " ", html.unescape(value)).strip()


class DocumentParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.title_parts: list[str] = []
        self.in_title = False
        self.current_heading: dict[str, Any] | None = None
        self.headings: list[dict[str, Any]] = []
        self.metas: list[dict[str, str]] = []
        self.links: list[dict[str, str]] = []
        self.images: list[dict[str, str]] = []
        self.canonicals: list[str] = []
        self.html_lang = ""
        self.json_ld_raw: list[str] = []
        self.in_json_ld = False
        self.json_ld_parts: list[str] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        values = {key.lower(): value or "" for key, value in attrs}
        tag = tag.lower()
        if tag == "html":
            self.html_lang = values.get("lang", "")
        elif tag == "title":
            self.in_title = True
        elif tag == "meta":
            self.metas.append(values)
        elif tag == "link":
            rel = {part.lower() for part in values.get("rel", "").split()}
            if "canonical" in rel and values.get("href"):
                self.canonicals.append(values["href"])
            self.links.append(values)
        elif tag == "a":
            self.links.append(values)
        elif tag == "img":
            self.images.append(values)
        elif re.fullmatch(r"h[1-6]", tag):
            self.current_heading = {"level": int(tag[1]), "parts": []}
        elif tag == "script" and values.get("type", "").lower() == "application/ld+json":
            self.in_json_ld = True
            self.json_ld_parts = []

    def handle_endtag(self, tag: str) -> None:
        tag = tag.lower()
        if tag == "title":
            self.in_title = False
        elif re.fullmatch(r"h[1-6]", tag) and self.current_heading:
            self.headings.append(
                {
                    "level": self.current_heading["level"],
                    "text": clean_text("".join(self.current_heading["parts"])),
                }
            )
            self.current_heading = None
        elif tag == "script" and self.in_json_ld:
            self.json_ld_raw.append("".join(self.json_ld_parts).strip())
            self.in_json_ld = False
            self.json_ld_parts = []

    def handle_data(self, data: str) -> None:
        if self.in_title:
            self.title_parts.append(data)
        if self.current_heading:
            self.current_heading["parts"].append(data)
        if self.in_json_ld:
            self.json_ld_parts.append(data)


def fetch(url: str, timeout: float, max_bytes: int) -> dict[str, Any]:
    request = Request(url, headers={"User-Agent": USER_AGENT, "Accept": "*/*"})
    try:
        with urlopen(request, timeout=timeout) as response:
            body = response.read(max_bytes + 1)
            truncated = len(body) > max_bytes
            if truncated:
                body = body[:max_bytes]
            charset = response.headers.get_content_charset() or "utf-8"
            return {
                "ok": True,
                "status": response.status,
                "url": response.geturl(),
                "contentType": response.headers.get("Content-Type", ""),
                "headers": dict(response.headers.items()),
                "body": body.decode(charset, errors="replace"),
                "truncated": truncated,
            }
    except HTTPError as error:
        body = error.read(max_bytes).decode("utf-8", errors="replace")
        return {
            "ok": False,
            "status": error.code,
            "url": error.geturl(),
            "contentType": error.headers.get("Content-Type", ""),
            "headers": dict(error.headers.items()),
            "body": body,
            "truncated": False,
            "error": str(error),
        }
    except (URLError, TimeoutError, OSError) as error:
        return {"ok": False, "status": None, "url": url, "error": str(error), "body": ""}


def finding(code: str, severity: str, message: str, evidence: Any = None) -> dict[str, Any]:
    item = {"code": code, "severity": severity, "message": message}
    if evidence not in (None, "", [], {}):
        item["evidence"] = evidence
    return item


def meta_values(parser: DocumentParser, key: str, value: str) -> list[str]:
    return [item.get("content", "") for item in parser.metas if item.get(key, "").lower() == value]


def json_ld_types(value: Any) -> list[str]:
    found: list[str] = []
    if isinstance(value, dict):
        raw_type = value.get("@type")
        if isinstance(raw_type, str):
            found.append(raw_type)
        elif isinstance(raw_type, list):
            found.extend(item for item in raw_type if isinstance(item, str))
        for nested in value.values():
            found.extend(json_ld_types(nested))
    elif isinstance(value, list):
        for nested in value:
            found.extend(json_ld_types(nested))
    return found


def audit_page(requested_url: str, response: dict[str, Any]) -> dict[str, Any]:
    findings: list[dict[str, Any]] = []
    status = response.get("status")
    if not response.get("ok"):
        findings.append(finding("http.fetch", "error", "Page could not be fetched successfully.", response.get("error")))
    elif not (200 <= status < 300):
        findings.append(finding("http.status", "error", f"Expected a successful response, received {status}."))
    if response.get("truncated"):
        findings.append(finding("http.truncated", "warning", "HTML exceeded the configured byte limit; results may be incomplete."))
    content_type = response.get("contentType", "")
    if content_type and "html" not in content_type.lower():
        findings.append(finding("http.content-type", "error", "Response is not identified as HTML.", content_type))

    parser = DocumentParser()
    parser.feed(response.get("body", ""))
    final_url = response.get("url", requested_url)
    title = clean_text("".join(parser.title_parts))
    descriptions = meta_values(parser, "name", "description")
    robots = [value.lower() for value in meta_values(parser, "name", "robots")]
    response_headers = {key.lower(): value for key, value in response.get("headers", {}).items()}
    x_robots = [response_headers["x-robots-tag"].lower()] if response_headers.get("x-robots-tag") else []
    og = {
        name: meta_values(parser, "property", name)
        for name in ("og:title", "og:type", "og:image", "og:url", "og:description", "og:image:alt")
    }

    if not title:
        findings.append(finding("metadata.title.missing", "error", "Document has no non-empty title."))
    if len(descriptions) != 1 or not clean_text(descriptions[0] if descriptions else ""):
        findings.append(finding("metadata.description", "error", "Expected one non-empty meta description.", descriptions))
    if len(parser.canonicals) != 1:
        findings.append(finding("canonical.count", "error", "Expected exactly one canonical link.", parser.canonicals))
    else:
        if urlparse(parser.canonicals[0]).scheme not in ("http", "https"):
            findings.append(finding("canonical.absolute-url", "warning", "Canonical should use an absolute HTTP(S) URL.", parser.canonicals[0]))
        canonical = urljoin(final_url, parser.canonicals[0])
        if canonical != final_url:
            findings.append(finding("canonical.target", "warning", "Canonical differs from the fetched final URL; verify that consolidation is intentional.", {"canonical": canonical, "finalUrl": final_url}))
    robots_tokens = {token.strip() for value in [*robots, *x_robots] for token in value.split(",")}
    if "noindex" in robots_tokens:
        findings.append(finding("robots.noindex", "error", "The audited public URL declares noindex.", {"meta": robots, "headers": x_robots}))
    if not parser.html_lang:
        findings.append(finding("html.lang", "warning", "The html element has no language declaration."))

    h1_count = sum(item["level"] == 1 for item in parser.headings)
    if h1_count == 0:
        findings.append(finding("headings.main-topic", "warning", "No h1 identifies the page's main topic."))
    elif h1_count > 1:
        findings.append(finding("headings.multiple-h1", "info", "Multiple h1 elements found; confirm that the document remains semantically clear.", h1_count))
    previous = 0
    for heading in parser.headings:
        if previous and heading["level"] > previous + 1:
            findings.append(finding("headings.skipped-level", "warning", "Heading hierarchy skips a level.", heading))
        previous = heading["level"]

    for required in ("og:title", "og:type", "og:image", "og:url"):
        if len(og[required]) != 1 or not clean_text(og[required][0] if og[required] else ""):
            findings.append(finding("open-graph." + required[3:].replace(":", "-"), "error", f"Expected one non-empty {required} property.", og[required]))
    for property_name in ("og:image", "og:url"):
        if og[property_name] and urlparse(og[property_name][0]).scheme not in ("http", "https"):
            findings.append(finding("open-graph.absolute-url", "error", f"{property_name} must use an absolute HTTP(S) URL.", og[property_name][0]))
    if og["og:url"] and urljoin(final_url, og["og:url"][0]) != final_url:
        findings.append(finding("open-graph.url-mismatch", "warning", "og:url differs from the fetched final URL.", {"og:url": og["og:url"][0], "finalUrl": final_url}))
    if og["og:image"] and not og["og:image:alt"]:
        findings.append(finding("open-graph.image-alt", "warning", "Open Graph image has no og:image:alt."))

    missing_alt = [image.get("src", "") for image in parser.images if "alt" not in image]
    missing_dimensions = [image.get("src", "") for image in parser.images if not image.get("width") or not image.get("height")]
    if missing_alt:
        findings.append(finding("images.alt", "warning", "Images without an alt attribute found.", missing_alt))
    if missing_dimensions:
        findings.append(finding("images.dimensions", "warning", "Images without explicit width and height found.", missing_dimensions))

    parsed_json_ld: list[Any] = []
    invalid_json_ld: list[str] = []
    for raw in parser.json_ld_raw:
        try:
            parsed_json_ld.append(json.loads(raw))
        except json.JSONDecodeError as error:
            invalid_json_ld.append(str(error))
    if invalid_json_ld:
        findings.append(finding("json-ld.invalid", "error", "Invalid JSON-LD block found.", invalid_json_ld))
    if not parser.json_ld_raw:
        findings.append(finding("json-ld.absent", "info", "No JSON-LD found; add it only when an applicable type matches visible content."))

    internal_links = 0
    origin = urlparse(final_url)
    for link in parser.links:
        href = link.get("href", "")
        if not href or href.startswith(("#", "mailto:", "tel:", "javascript:")):
            continue
        target = urlparse(urljoin(final_url, href))
        if target.netloc == origin.netloc:
            internal_links += 1
    if internal_links == 0:
        findings.append(finding("links.internal", "warning", "No crawlable internal links were found."))

    return {
        "requestedUrl": requested_url,
        "finalUrl": final_url,
        "status": status,
        "contentType": content_type,
        "title": title,
        "description": descriptions[0] if descriptions else None,
        "canonical": urljoin(final_url, parser.canonicals[0]) if len(parser.canonicals) == 1 else None,
        "robots": {"meta": robots, "headers": x_robots},
        "openGraph": {key: values for key, values in og.items()},
        "headings": parser.headings,
        "images": {"count": len(parser.images), "missingAlt": missing_alt, "missingDimensions": missing_dimensions},
        "internalLinkCount": internal_links,
        "jsonLd": {"blocks": len(parser.json_ld_raw), "types": sorted(set(json_ld_types(parsed_json_ld))), "invalid": invalid_json_ld},
        "findings": findings,
    }


def audit_site_files(urls: list[str], timeout: float, max_bytes: int) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for origin in sorted({f"{urlparse(url).scheme}://{urlparse(url).netloc}" for url in urls}):
        robots_url = urljoin(origin, "/robots.txt")
        robots = fetch(robots_url, timeout, max_bytes)
        sitemap_urls: list[str] = []
        external_sitemaps: list[str] = []
        if robots.get("ok"):
            for line in robots["body"].splitlines():
                if not line.lower().startswith("sitemap:"):
                    continue
                sitemap_url = urljoin(origin, line.split(":", 1)[1].strip())
                if urlparse(sitemap_url).netloc == urlparse(origin).netloc:
                    sitemap_urls.append(sitemap_url)
                else:
                    external_sitemaps.append(sitemap_url)
        if not sitemap_urls:
            sitemap_urls = [urljoin(origin, "/sitemap.xml")]
        sitemap_results = []
        for sitemap_url in sitemap_urls:
            response = fetch(sitemap_url, timeout, max_bytes)
            xml_valid = False
            xml_error = None
            if response.get("ok"):
                try:
                    ET.fromstring(response.get("body", ""))
                    xml_valid = True
                except ET.ParseError as error:
                    xml_error = str(error)
            sitemap_results.append({"url": sitemap_url, "status": response.get("status"), "validXml": xml_valid, "error": xml_error or response.get("error")})
        llms_url = urljoin(origin, "/llms.txt")
        llms = fetch(llms_url, timeout, max_bytes)
        results.append(
            {
                "origin": origin,
                "robots": {"url": robots_url, "status": robots.get("status"), "body": robots.get("body", "") if robots.get("ok") else None, "error": robots.get("error")},
                "sitemaps": sitemap_results,
                "externalSitemaps": external_sitemaps,
                "llmsTxt": {"url": llms_url, "status": llms.get("status"), "present": bool(llms.get("ok")), "required": False},
            }
        )
    return results


def audit_local_site_files(root: Path, public_url: str) -> list[dict[str, Any]]:
    parsed_public = urlparse(public_url)
    origin = f"{parsed_public.scheme}://{parsed_public.netloc}"
    robots_path = root / "robots.txt"
    robots_body = robots_path.read_text(errors="replace") if robots_path.is_file() else ""
    declared = [line.split(":", 1)[1].strip() for line in robots_body.splitlines() if line.lower().startswith("sitemap:")]
    sitemap_paths: list[Path] = []
    for sitemap_url in declared:
        parsed = urlparse(sitemap_url)
        if parsed.netloc and parsed.netloc != parsed_public.netloc:
            continue
        candidate = (root / parsed.path.lstrip("/")).resolve()
        if candidate != root and root not in candidate.parents:
            continue
        if candidate not in sitemap_paths:
            sitemap_paths.append(candidate)
    if not sitemap_paths:
        sitemap_paths.append(root / "sitemap.xml")
    sitemap_results = []
    for sitemap_path in sitemap_paths:
        valid = False
        error = None
        if sitemap_path.is_file():
            try:
                ET.fromstring(sitemap_path.read_text(errors="replace"))
                valid = True
            except ET.ParseError as parse_error:
                error = str(parse_error)
        sitemap_results.append(
            {
                "url": urljoin(origin, "/" + str(sitemap_path.relative_to(root))),
                "status": 200 if sitemap_path.is_file() else 404,
                "validXml": valid,
                "error": error,
            }
        )
    llms_path = root / "llms.txt"
    return [
        {
            "origin": origin,
            "robots": {
                "url": urljoin(origin, "/robots.txt"),
                "status": 200 if robots_path.is_file() else 404,
                "body": robots_body if robots_path.is_file() else None,
                "error": None,
            },
            "sitemaps": sitemap_results,
            "externalSitemaps": [url for url in declared if urlparse(url).netloc and urlparse(url).netloc != parsed_public.netloc],
            "llmsTxt": {
                "url": urljoin(origin, "/llms.txt"),
                "status": 200 if llms_path.is_file() else 404,
                "present": llms_path.is_file(),
                "required": False,
            },
        }
    ]


def generic_robots_blocks_all(body: str) -> bool:
    agents: list[str] = []
    saw_directive = False
    for raw_line in body.splitlines():
        line = raw_line.split("#", 1)[0].strip()
        if not line or ":" not in line:
            continue
        key, value = (part.strip() for part in line.split(":", 1))
        key = key.lower()
        if key == "user-agent":
            if saw_directive:
                agents = []
                saw_directive = False
            agents.append(value.lower())
            continue
        saw_directive = True
        if key == "disallow" and "*" in agents and value == "/":
            return True
    return False


def attach_site_file_findings(site: dict[str, Any]) -> None:
    findings: list[dict[str, Any]] = []
    robots = site["robots"]
    if robots["status"] == 404:
        findings.append(finding("robots-txt.absent", "info", "robots.txt is absent; this is valid when default crawl behavior is intended."))
    elif robots["status"] != 200:
        findings.append(finding("robots-txt.fetch", "warning", "robots.txt did not return HTTP 200.", robots["status"]))
    elif generic_robots_blocks_all(robots.get("body") or ""):
        findings.append(finding("robots-txt.blocks-all", "error", "The generic robots.txt group disallows the entire site."))
    for sitemap in site["sitemaps"]:
        if sitemap["status"] == 404:
            findings.append(finding("sitemap.absent", "info", "No sitemap was found at the discovered location.", sitemap["url"]))
        elif sitemap["status"] != 200:
            findings.append(finding("sitemap.fetch", "warning", "Sitemap did not return HTTP 200.", {"url": sitemap["url"], "status": sitemap["status"]}))
        elif not sitemap["validXml"]:
            findings.append(finding("sitemap.invalid-xml", "error", "Sitemap is not valid XML.", {"url": sitemap["url"], "error": sitemap["error"]}))
    if site.get("externalSitemaps"):
        findings.append(finding("sitemap.external", "warning", "External sitemap declarations were recorded but not fetched automatically.", site["externalSitemaps"]))
    if not site["llmsTxt"]["present"]:
        findings.append(finding("llms-txt.absent", "info", "llms.txt is absent; it is experimental and not required."))
    site["findings"] = findings


def attach_duplicate_metadata_findings(pages: list[dict[str, Any]]) -> None:
    for field, code, label in (
        ("title", "metadata.title.duplicate", "title"),
        ("description", "metadata.description.duplicate", "meta description"),
        ("canonical", "canonical.duplicate", "canonical URL"),
    ):
        grouped: dict[str, list[dict[str, Any]]] = {}
        for page in pages:
            value = clean_text(page.get(field) or "")
            if value:
                grouped.setdefault(value, []).append(page)
        for value, duplicates in grouped.items():
            if len(duplicates) < 2:
                continue
            urls = [page["requestedUrl"] for page in duplicates]
            for page in duplicates:
                page["findings"].append(finding(code, "warning", f"The same {label} appears on multiple audited URLs.", {"value": value, "urls": urls}))


def markdown_report(report: dict[str, Any]) -> str:
    lines = ["# Website SEO Finalization Audit", "", f"Generated: `{report['generatedAt']}`", ""]
    lines.extend(["## Summary", "", f"- URLs: {len(report['pages'])}", f"- Errors: {report['summary']['error']}", f"- Warnings: {report['summary']['warning']}", f"- Informational: {report['summary']['info']}", ""])
    for page in report["pages"]:
        lines.extend([f"## {page['requestedUrl']}", "", f"- Status: `{page['status']}`", f"- Final URL: `{page['finalUrl']}`", f"- Title: {page['title'] or '_missing_'}", f"- Canonical: `{page['canonical'] or 'missing'}`", "", "| Severity | Code | Finding |", "| --- | --- | --- |"])
        if not page["findings"]:
            lines.append("| - | - | No automated findings |")
        for item in page["findings"]:
            message = item["message"].replace("|", "\\|")
            lines.append(f"| {item['severity']} | `{item['code']}` | {message} |")
        lines.append("")
    lines.extend(["## Site files", ""])
    for site in report["siteFiles"]:
        lines.append(f"- `{site['origin']}`: robots `{site['robots']['status']}`, llms.txt `{site['llmsTxt']['status']}` (optional)")
        for sitemap in site["sitemaps"]:
            lines.append(f"  - sitemap `{sitemap['url']}`: status `{sitemap['status']}`, valid XML `{sitemap['validXml']}`")
        for item in site.get("findings", []):
            lines.append(f"  - {item['severity']}: `{item['code']}` — {item['message']}")
    lines.extend(["", "> Automated findings require contextual review. A clean report does not prove indexing, rankings, rich results, or LLM citations.", ""])
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", action="append", required=True, help="Public or locally served URL; repeat for multiple pages")
    parser.add_argument("--html-file", action="append", help="Generated HTML paired by order with --url; repeat for multiple pages")
    parser.add_argument("--site-root", help="Validate local robots.txt, sitemap.xml, and optional llms.txt")
    parser.add_argument("--output-dir", default="seo-audit", help="Directory for report.json and report.md")
    parser.add_argument("--timeout", type=float, default=15.0)
    parser.add_argument("--max-bytes", type=int, default=5_000_000)
    parser.add_argument("--skip-site-files", action="store_true")
    parser.add_argument("--fail-on", choices=("error", "warning", "never"), default="error")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.html_file and len(args.url) != len(args.html_file):
        raise SystemExit("Provide one --html-file for each --url")
    if args.site_root and not args.html_file:
        raise SystemExit("--site-root is supported with --html-file")
    for url in args.url:
        parsed = urlparse(url)
        if parsed.scheme not in ("http", "https") or not parsed.netloc:
            raise SystemExit(f"Invalid HTTP(S) URL: {url}")
    if args.site_root and len({urlparse(url).netloc for url in args.url}) != 1:
        raise SystemExit("--site-root requires all URLs to share one origin")
    if args.html_file:
        pages = []
        for url, file_name in zip(args.url, args.html_file, strict=True):
            html_path = Path(file_name).resolve()
            response = {
                "ok": True,
                "status": 200,
                "url": url,
                "contentType": "text/html; charset=utf-8",
                "headers": {},
                "body": html_path.read_text(errors="replace"),
                "truncated": False,
            }
            pages.append(audit_page(url, response))
    else:
        pages = [audit_page(url, fetch(url, args.timeout, args.max_bytes)) for url in args.url]
    if args.skip_site_files:
        site_files = []
    elif args.site_root:
        site_files = audit_local_site_files(Path(args.site_root).resolve(), args.url[0])
    else:
        site_files = audit_site_files(args.url, args.timeout, args.max_bytes)
    for site in site_files:
        attach_site_file_findings(site)
    attach_duplicate_metadata_findings(pages)
    counts = {severity: 0 for severity in SEVERITY_RANK}
    for page in pages:
        for item in page["findings"]:
            counts[item["severity"]] += 1
    for site in site_files:
        for item in site.get("findings", []):
            counts[item["severity"]] += 1
    from datetime import datetime, timezone

    report = {"schemaVersion": 1, "generatedAt": datetime.now(timezone.utc).isoformat(), "pages": pages, "siteFiles": site_files, "summary": counts}
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "report.json").write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n")
    (output_dir / "report.md").write_text(markdown_report(report))
    print(f"Wrote {output_dir / 'report.json'}")
    print(f"Wrote {output_dir / 'report.md'}")
    if args.fail_on == "never":
        return 0
    threshold = SEVERITY_RANK[args.fail_on]
    all_findings = [item for page in pages for item in page["findings"]] + [item for site in site_files for item in site.get("findings", [])]
    return 2 if any(SEVERITY_RANK[item["severity"]] >= threshold for item in all_findings) else 0


if __name__ == "__main__":
    sys.exit(main())
