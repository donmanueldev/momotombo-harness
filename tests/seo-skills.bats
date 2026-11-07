#!/usr/bin/env bats

setup_file() {
    cargo build --release --quiet --manifest-path "$BATS_TEST_DIRNAME/../tools/Cargo.toml"
}

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TOOLS="$PROJECT_DIR/tools/target/release"
    TEST_TMP="$(mktemp -d)"
}

teardown() {
    rm -rf "$TEST_TMP"
}

create_fake_lighthouse() {
    cat > "$TEST_TMP/lighthouse" <<'SH'
#!/bin/bash
set -euo pipefail
if [[ "${1:-}" == "--version" ]]; then
    echo "99.0.0-test"
    exit 0
fi
output=""
for argument in "$@"; do
    case "$argument" in
        --output-path=*) output="${argument#--output-path=}" ;;
    esac
done
[[ -n "$output" ]]
run_number="$(basename "$output" .json | sed 's/run-//')"
lcp=$((run_number * 1000))
cat > "$output" <<JSON
{
  "fetchTime": "2026-01-01T00:00:00.000Z",
  "finalUrl": "https://example.test/",
  "categories": {"performance": {"score": 0.92}},
  "audits": {
    "first-contentful-paint": {"numericValue": 900},
    "largest-contentful-paint": {"numericValue": $lcp},
    "speed-index": {"numericValue": 1200},
    "total-blocking-time": {"numericValue": 100},
    "cumulative-layout-shift": {"numericValue": 0.05},
    "total-byte-weight": {"numericValue": 500000}
  }
}
JSON
SH
    chmod +x "$TEST_TMP/lighthouse"
}

@test "web adapter detects Next.js and its package manager" {
    mkdir -p "$TEST_TMP/next"
    cat > "$TEST_TMP/next/package.json" <<'JSON'
{"scripts":{"build":"next build","start":"next start"},"dependencies":{"next":"latest","react":"latest"}}
JSON
    touch "$TEST_TMP/next/bun.lock"

    run "$TOOLS/detect-web-adapter" --root "$TEST_TMP/next"

    [[ "$status" -eq 0 ]]
    [[ "$output" == *'"framework": "nextjs"'* ]]
    [[ "$output" == *'"packageManager": "bun"'* ]]
    [[ "$output" == *'"start"'* ]]
}

@test "web adapter distinguishes Astro React Vite and static HTML" {
    mkdir -p "$TEST_TMP/astro" "$TEST_TMP/vite" "$TEST_TMP/static"
    cat > "$TEST_TMP/astro/package.json" <<'JSON'
{"scripts":{"build":"astro build","preview":"astro preview"},"devDependencies":{"astro":"latest"}}
JSON
    cat > "$TEST_TMP/vite/package.json" <<'JSON'
{"scripts":{"build":"vite build","preview":"vite preview"},"dependencies":{"react":"latest"},"devDependencies":{"vite":"latest"}}
JSON
    touch "$TEST_TMP/static/index.html"

    run "$TOOLS/detect-web-adapter" --root "$TEST_TMP/astro"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *'"framework": "astro"'* ]]

    run "$TOOLS/detect-web-adapter" --root "$TEST_TMP/vite"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *'"framework": "react-vite"'* ]]

    run "$TOOLS/detect-web-adapter" --root "$TEST_TMP/static"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *'"framework": "static-html"'* ]]
}

@test "SEO auditor writes JSON and Markdown for rendered URLs" {
    mkdir -p "$TEST_TMP/site"
    cat > "$TEST_TMP/site/index.html" <<'HTML'
<!doctype html>
<html lang="en"><head>
<title>Example product</title>
<meta name="description" content="A useful example product page.">
<link rel="canonical" href="https://example.test/">
<meta property="og:title" content="Example product">
<meta property="og:type" content="website">
<meta property="og:image" content="https://example.test/image.png">
<meta property="og:image:alt" content="Example product preview">
<meta property="og:url" content="https://example.test/">
<script type="application/ld+json">{"@context":"https://schema.org","@type":"WebSite","name":"Example"}</script>
</head><body><h1>Example product</h1><a href="/about">About</a><img src="/image.png" alt="Product" width="1200" height="630"></body></html>
HTML
    cat > "$TEST_TMP/site/robots.txt" <<'ROBOTS'
User-agent: *
Allow: /
Sitemap: https://example.test/sitemap.xml
ROBOTS
    cat > "$TEST_TMP/site/sitemap.xml" <<'XML'
<?xml version="1.0" encoding="UTF-8"?><urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9"><url><loc>https://example.test/</loc></url></urlset>
XML

    run "$TOOLS/audit-site" --url "https://example.test/" --html-file "$TEST_TMP/site/index.html" --site-root "$TEST_TMP/site" --output-dir "$TEST_TMP/report" --fail-on error

    [[ "$status" -eq 0 ]]
    [[ -f "$TEST_TMP/report/report.json" ]]
    [[ -f "$TEST_TMP/report/report.md" ]]
    run jq -e '.summary.error == 0 and .pages[0].jsonLd.types == ["WebSite"] and .siteFiles[0].sitemaps[0].validXml' "$TEST_TMP/report/report.json"
    [[ "$status" -eq 0 ]]
}

@test "SEO auditor fails CI for technical errors" {
    mkdir -p "$TEST_TMP/broken"
    cat > "$TEST_TMP/broken/index.html" <<'HTML'
<!doctype html><html><head><title></title><meta name="robots" content="noindex"></head><body><h1>Broken page</h1></body></html>
HTML
    cat > "$TEST_TMP/broken/robots.txt" <<'ROBOTS'
User-agent: *
Disallow: /
ROBOTS

    run "$TOOLS/audit-site" --url https://example.test/ --html-file "$TEST_TMP/broken/index.html" --site-root "$TEST_TMP/broken" --output-dir "$TEST_TMP/broken-report" --fail-on error

    [[ "$status" -eq 2 ]]
    run jq -e '([.pages[].findings[].code] | index("metadata.title.missing") and index("robots.noindex")) and ([.siteFiles[].findings[].code] | index("robots-txt.blocks-all")) and .summary.error >= 3' "$TEST_TMP/broken-report/report.json"
    [[ "$status" -eq 0 ]]
}

@test "SEO auditor detects duplicate metadata across generated routes" {
    mkdir -p "$TEST_TMP/duplicates"
    for page in one two; do
        cat > "$TEST_TMP/duplicates/$page.html" <<HTML
<!doctype html><html lang="en"><head><title>Shared title</title><meta name="description" content="Shared description"><link rel="canonical" href="https://example.test/shared"><meta property="og:title" content="Shared title"><meta property="og:type" content="website"><meta property="og:image" content="https://example.test/image.png"><meta property="og:url" content="https://example.test/$page"></head><body><h1>$page</h1><a href="/">Home</a></body></html>
HTML
    done

    run "$TOOLS/audit-site" --url https://example.test/one --html-file "$TEST_TMP/duplicates/one.html" --url https://example.test/two --html-file "$TEST_TMP/duplicates/two.html" --skip-site-files --output-dir "$TEST_TMP/duplicate-report" --fail-on error

    [[ "$status" -eq 0 ]]
    run jq -e '[.pages[].findings[].code] | (index("metadata.title.duplicate") and index("metadata.description.duplicate") and index("canonical.duplicate"))' "$TEST_TMP/duplicate-report/report.json"
    [[ "$status" -eq 0 ]]
}

@test "Lighthouse runner calculates a median across repeated runs" {
    create_fake_lighthouse
    cat > "$TEST_TMP/budget.json" <<'JSON'
{"default":{"performanceScore":{"min":0.9}},"mobile":{"largestContentfulPaintMs":{"max":2500}},"desktop":{"largestContentfulPaintMs":{"max":1500}}}
JSON

    run "$TOOLS/run-lighthouse" --url https://example.test/ --profiles mobile --runs 3 --lighthouse-bin "$TEST_TMP/lighthouse" --budget "$TEST_TMP/budget.json" --output-dir "$TEST_TMP/lighthouse-report"

    [[ "$status" -eq 0 ]]
    run jq -e '.profiles.mobile.median.largestContentfulPaintMs == 2000 and .passed == true and (.profiles.mobile.runs | length) == 3' "$TEST_TMP/lighthouse-report/report.json"
    [[ "$status" -eq 0 ]]
}

@test "Lighthouse runner exits nonzero when a performance budget fails" {
    create_fake_lighthouse
    cat > "$TEST_TMP/budget.json" <<'JSON'
{"largestContentfulPaintMs":{"max":1500}}
JSON

    run "$TOOLS/run-lighthouse" --url https://example.test/ --profiles desktop --runs 3 --lighthouse-bin "$TEST_TMP/lighthouse" --budget "$TEST_TMP/budget.json" --output-dir "$TEST_TMP/lighthouse-report"

    [[ "$status" -eq 2 ]]
    run jq -e '.passed == false and .profiles.desktop.budgetResults[0].passed == false' "$TEST_TMP/lighthouse-report/report.json"
    [[ "$status" -eq 0 ]]
}

@test "skills expose the Rust tools through bin symlinks" {
    [[ -x "$PROJECT_DIR/skills/pagespeed-optimization/bin/detect-web-adapter" ]]
    [[ -x "$PROJECT_DIR/skills/pagespeed-optimization/bin/run-lighthouse" ]]
    [[ -x "$PROJECT_DIR/skills/website-seo-finalization/bin/audit-site" ]]
    run "$PROJECT_DIR/skills/website-seo-finalization/bin/audit-site" --help
    [[ "$status" -eq 0 ]]
}

@test "repository ships no Python sources" {
    run bash -c "cd '$PROJECT_DIR' && git ls-files '*.py' && grep -rlE 'python3? ' --include='*.sh' --include='*.md' --include='*.bats' skills tests install.sh sync.sh | grep -v seo-skills.bats"
    [[ -z "$output" ]]
}
