---
name: website-seo-finalization
description: Finalize a public website for launch by auditing and, when requested, correcting SEO, AEO and GEO readiness, indexability, metadata, Open Graph previews, structured data, answer and citation clarity, crawler access, headings, links, images, sitemaps, and robots directives. Use for pre-launch search or generative-discovery readiness; do not use for paid acquisition or general performance tuning.
category: seo
version: 2.0.0
status: stable
---

# Website SEO Finalization

Prepare each intended public URL for discovery and accurate presentation. Treat
observable defects, contextual recommendations, and optional heuristics as
different things; do not promise rankings or indexing.

## Establish the release surface

Before changing code:

1. Read the repository instructions and identify the framework, metadata entry
   points, build command, canonical production origin, and intended public routes.
2. Distinguish indexable pages from private, authenticated, utility, duplicate,
   preview, staging, and error routes.
3. Inspect rendered HTML when possible. Source declarations alone do not prove
   the deployed tags, response headers, or crawler-visible content.
4. If the request is an audit, remain read-only. Implement corrections only when
   the user requested changes.

## Audit by URL

For every important indexable URL, verify:

- a successful response and crawler-accessible primary content;
- a useful, page-specific title and meta description;
- a self-consistent canonical URL and non-contradictory robots directives;
- one clearly identifiable main topic with semantic heading structure;
- crawlable internal links to and from related public pages;
- accurate Open Graph data and a representative, publicly fetchable image;
- structured data only when an applicable type matches visible content;
- meaningful image alternatives, intrinsic dimensions, and appropriate delivery;
- inclusion in a sitemap when a sitemap materially helps discovery; and
- coherent `robots.txt`, sitemap, meta robots, and `X-Robots-Tag` behavior;
- answer-ready passages that are clear, accurate, attributable, and useful without
  artificial repetition or filler; and
- intended eligibility for search and generative crawlers without conflating
  search retrieval, user-triggered fetches, model training, or AI-agent access.

Read [Technical SEO](references/technical-seo.md) for crawler controls, metadata,
headings, image decisions, sitemaps, and the optional `llms.txt` check. Read
[Social previews and structured data](references/social-and-structured-data.md)
when reviewing Open Graph, preview tooling, or schema markup. Read
[AEO and GEO readiness](references/aeo-and-geo.md) when auditing direct answers,
generative citations, AI crawler policy, entity consistency, or LLM visibility.

## Interpret findings correctly

Classify every reported item:

- **Technical error:** verified behavior that prevents or contradicts the intended
  crawl, index, canonical, preview, accessibility, or structured-data outcome.
- **Contextual recommendation:** an improvement supported by this site's content,
  audience, or measured behavior, but not a universal requirement.
- **Optional heuristic:** a convention or third-party score that may be useful but
  must not block release on its own.

Never report these as universal SEO errors:

- exceeding a fixed title or description character count;
- having more than one `h1` when the document remains semantically clear;
- omitting `meta keywords`, which Google ignores;
- using an appropriate image format other than WebP;
- omitting a sitemap from a small, fully crawlable site;
- lacking `llms.txt`, which is experimental and not a Google Search requirement;
- lacking a special "AI schema", because no universal AEO/GEO schema exists; or
- failing a single prompt, generated answer, citation check, or vendor score.

## Validate and report

For repeatable URL-level checks, run the bundled zero-dependency auditor:

```bash
<skill-path>/bin/audit-site \
  --url https://example.com/ \
  --url https://example.com/product/ \
  --output-dir <artifact-directory> \
  --fail-on error
```

It writes machine-readable `report.json` and human-readable `report.md`, checks
rendered response HTML plus origin-level robots, sitemap, and optional `llms.txt`,
and exits nonzero at the configured severity. It intentionally does not crawl the
whole site, execute client JavaScript, judge content quality, or prove indexing.
For client-rendered React, audit the production deployment or a production build
served locally; source JSX is not rendered evidence.

For Astro, static React output, or plain HTML, audit generated files without a
server when network access is unavailable:

```bash
<skill-path>/bin/audit-site \
  --url https://example.com/ \
  --html-file <output-directory>/index.html \
  --site-root <output-directory> \
  --output-dir <artifact-directory>
```

Repeat `--url` and `--html-file` in matching order to audit multiple generated
routes. The auditor also flags duplicate titles, descriptions, and canonicals
across the supplied route set.

Run the project's existing build and lint checks. Where the environment permits,
inspect representative rendered pages, response headers, `robots.txt`, sitemap
output, structured-data validation, social previews, and explicitly requested AI
search surfaces. Record the engine, model or product surface, date, locale, query,
account state, and visible citations for nondeterministic answer checks. Use
[OpenGraph.to](https://www.opengraph.to/) only for a public URL and only as
supplementary evidence; do not upload or expose private preview URLs.

Report:

- URLs and release surface inspected;
- changes made, grouped by technical error or recommendation;
- exact validation performed and its environment;
- SEO, AEO, and GEO evidence separately, without treating the labels as separate
  guaranteed ranking systems;
- third-party checks separately from source/build/deployed evidence; and
- remaining blockers, optional improvements, and anything requiring production
  access or post-deploy verification.
