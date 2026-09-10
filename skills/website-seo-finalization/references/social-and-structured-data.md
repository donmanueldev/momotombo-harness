# Social Previews and Structured Data

## Open Graph

For an indexable public page, inspect the rendered values and resolved URLs for:

- required protocol properties: `og:title`, `og:type`, `og:image`, and `og:url`;
- a useful `og:description` where it improves the preview;
- `og:image:alt` for images that convey information; and
- image dimensions, MIME type, public fetchability, and representative content.

Keep preview content consistent with the visible page. Prefer absolute URLs and
verify that production robots, authentication, or hotlink protection do not block
the image. Add platform-specific metadata only when the target platform requires
behavior not supplied by its Open Graph fallback.

[The Open Graph protocol](https://ogp.me/) is the normative source for Open Graph
properties. [OpenGraph.to](https://www.opengraph.to/) is an optional public-URL
preview check. Its score, size recommendations, cache, and simulated preview are
heuristics; they do not prove how every network will render or refresh a card.

For localhost or a private preview, inspect rendered HTML and image assets
directly. Do not expose a private deployment merely to obtain a third-party score.

## Structured data

- Choose a type because it accurately represents visible content and an intended
  search feature, not because a schema exists.
- Prefer JSON-LD when it fits the framework, while recognizing that Google also
  supports Microdata and RDFa.
- Keep names, prices, availability, dates, ratings, authorship, organization data,
  and URLs consistent with the page users see.
- Validate syntax and eligibility separately. Valid markup does not guarantee a
  rich result, indexing, or ranking.
- Do not create ratings, reviews, FAQs, business attributes, or other claims that
  the visible product cannot substantiate.

Primary references:

- [Google structured-data policies](https://developers.google.com/search/docs/appearance/structured-data/sd-policies)
- [Schema.org vocabulary](https://schema.org/)
- [Google Rich Results Test](https://search.google.com/test/rich-results)
