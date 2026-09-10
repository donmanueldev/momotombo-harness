# Technical SEO Decision Guide

Use this reference for a final route-by-route crawl and indexability review.

## Crawl, index, and canonical behavior

- Confirm the final HTTP response, rendered primary content, redirect chain, and
  relevant `X-Robots-Tag` headers. A build passing is not deployed evidence.
- A canonical is a consolidation hint, not a redirect or an indexing guarantee.
  Keep it absolute and consistent with redirects, internal links, sitemap entries,
  protocol, host, locale, and trailing-slash policy.
- `robots.txt` controls crawling, not authorization. Do not use it to protect
  secrets or as a substitute for authentication or `noindex`.
- Do not add Google's unsupported `crawl-delay` directive.
- A URL blocked from crawling can prevent a crawler from seeing a page-level
  `noindex`. Avoid contradictory controls.

Primary references:

- [Google canonical guidance](https://developers.google.com/search/docs/crawling-indexing/consolidate-duplicate-urls)
- [Google robots.txt interpretation](https://developers.google.com/crawling/docs/robots-txt/robots-txt-spec)
- [Robots Exclusion Protocol, RFC 9309](https://www.rfc-editor.org/rfc/rfc9309)

## Titles, descriptions, headings, and links

- Make titles and descriptions accurate, useful, and page-specific. Treat common
  character ranges as preview heuristics because Google may rewrite or truncate
  them by query and device.
- Keep one obvious main topic and a logical heading outline. Exactly one `h1` is
  a maintainability and accessibility convention, not a universal ranking rule.
- Use real crawlable links for primary navigation. Avoid relying exclusively on
  scripted interactions that do not expose link destinations.
- Do not add `meta keywords`; Google Search ignores it.

Primary references:

- [Influencing title links](https://developers.google.com/search/docs/appearance/title-link)
- [Controlling snippets](https://developers.google.com/search/docs/appearance/snippet)
- [Supported and unsupported meta tags](https://developers.google.com/search/docs/crawling-indexing/special-tags)

## Images

- Choose AVIF, WebP, JPEG, PNG, SVG, or another supported format according to
  fidelity, transparency, animation, browser support, and measured transfer cost.
- Preserve visual quality. Supply intrinsic dimensions, responsive candidates,
  descriptive filenames where useful, and contextual alt text for informative
  images. Decorative images should not receive misleading descriptions.
- Do not lazy-load the likely LCP image. Reserve preload and `fetchpriority` for
  resources proven to be critical.

Primary references:

- [Google image SEO](https://developers.google.com/search/docs/appearance/google-images)
- [Choosing an image format](https://web.dev/articles/choose-the-right-image-format)

## Sitemap and `llms.txt`

- Use a sitemap when it improves discovery or communicates a large, new,
  media-heavy, or weakly linked URL set. It is not an indexing guarantee and may
  be unnecessary for a small, well-linked site.
- Include only canonical, intended indexable URLs and generate correct absolute
  locations. Validate XML and declared sitemap locations.
- Treat `/llms.txt` as an optional experimental artifact. If the product chooses
  to publish one, verify that its claims and links remain accurate; never make it
  an SEO, AEO, or GEO release gate or claim that Google Search uses it. Google
  states that its generative Search features do not require a new machine-readable
  file, AI text file, or special markup.

Primary references:

- [Google sitemap overview](https://developers.google.com/search/docs/crawling-indexing/sitemaps/overview)
- [Sitemaps protocol](https://www.sitemaps.org/protocol.html)
- [The llms.txt proposal](https://github.com/AnswerDotAI/llms-txt)
- [Google guidance for generative Search optimization](https://developers.google.com/search/docs/fundamentals/ai-optimization-guide)
