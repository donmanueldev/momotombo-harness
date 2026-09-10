# SEO and PageSpeed Launch

Use these copy-paste examples with `website-seo-finalization` and
`pagespeed-optimization`. Replace the placeholders before running them.

## Complete pre-launch optimization

```text
Use $website-seo-finalization and $pagespeed-optimization to prepare this
website for public launch.

Public URL: <URL, or "not available yet">
Priority routes: <routes, or "discover the important public routes">

Detect whether the project uses React, Next.js, Astro, or static HTML. Inspect
the current Git state and existing build, preview, test, Lighthouse, or
Lighthouse CI configuration before changing files.

Create an SEO/AEO/GEO and performance baseline, classify the findings by
severity, implement safe causal improvements, rebuild, and repeat equivalent
mobile and desktop measurements. Validate indexability, canonical URLs,
metadata, headings, internal links, Open Graph, JSON-LD, images, robots.txt,
sitemap.xml, and optional llms.txt. Preserve the intended design and behavior.

Do not pursue a Lighthouse score of 100 or treat meta keywords, exactly one H1,
universal WebP, rigid character limits, llms.txt, or a third-party preview score
as launch requirements. Do not commit, push, deploy, or install global
dependencies.

Finish with the implemented changes, modified files, before/after evidence,
validation results, residual risks, and checks that still require production or
field data. Distinguish verified facts from unavailable evidence.
```

## Read-only launch audit

```text
Use $website-seo-finalization and $pagespeed-optimization to audit this site
without modifying files.

URL: <PUBLIC_URL>
Priority routes: <ROUTES>

Generate JSON and Markdown reports for technical SEO, AEO/GEO readiness, Open
Graph, structured data, crawler controls, images, links, and mobile/desktop
performance. Use field Core Web Vitals when available and label Lighthouse as
laboratory evidence. Separate technical errors, contextual recommendations,
and optional heuristics.

Return a prioritized remediation plan with the affected routes and supporting
evidence. Do not edit, commit, push, deploy, or install dependencies.
```

## Implement improvements from an existing report

```text
Use $website-seo-finalization and $pagespeed-optimization to implement the
actionable findings in these reports:

SEO report: <PATH_TO_REPORT_JSON_OR_MD>
Lighthouse report: <PATH_TO_REPORT_JSON_OR_MD>
Public URL: <URL, or "not available yet">

Verify every finding against the current source before editing because the
report may be stale. Fix confirmed technical errors first, then implement only
contextually justified recommendations. Preserve intentional content, design,
analytics, and framework conventions.

Rebuild and rerun equivalent checks after the changes. Report fixed, rejected,
and unresolved findings with reasons and before/after evidence. Do not commit,
push, deploy, or make unrelated changes.
```

## Optimize one route or template

```text
Use $website-seo-finalization and $pagespeed-optimization to optimize only this
route and the shared code that directly renders it:

Route: <ROUTE_OR_URL>
Target audience and search intent: <AUDIENCE_AND_INTENT>
Important conversion: <PRIMARY_ACTION>

Measure the route before editing. Review its canonical, title, description,
heading structure, answer clarity, internal links, Open Graph, applicable
JSON-LD, images, LCP candidate, layout shifts, and interaction cost. Implement
the smallest causal improvements and measure again under equivalent conditions.

Do not rewrite unrelated pages or invent claims, keywords, FAQs, ratings, or
structured data unsupported by visible content. Return changed files,
before/after evidence, and remaining limitations.
```

## Add performance budgets to CI

```text
Use $pagespeed-optimization to add reproducible performance checks for this
project's existing CI workflow.

Critical URLs: <URLS>
CI provider: <PROVIDER>
Expected preview command: <COMMAND, or "detect it">

Inspect existing Lighthouse CI and package scripts first. Prefer the project's
current tooling when it already provides equivalent coverage. Establish a
documented baseline, propose realistic mobile and desktop budgets, then add the
smallest configuration needed to detect meaningful regressions. Keep lab
budgets separate from field Core Web Vitals and explain which failures block CI.

Do not download tools implicitly, weaken existing checks, or use a score of 100
as the contract. Validate the configuration locally where the environment
allows it and report anything that remains CI-only.
```

