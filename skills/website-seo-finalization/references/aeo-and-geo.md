# AEO and GEO Readiness

Use this reference when the launch review includes answer engines, AI search,
generated summaries, citations, or brand representation by retrieval-based LLMs.

## Working model

Treat the terms as useful audit views, not universal protocols:

- **SEO** establishes crawlability, indexability, relevance, and ordinary search
  presentation.
- **AEO** evaluates whether an eligible page answers real audience questions
  clearly enough to support direct-answer experiences.
- **GEO** evaluates whether generative search systems can retrieve, interpret,
  attribute, and accurately cite the site's public information.

These layers overlap. Google explicitly says that its generative Search features
use existing Search foundations, require no special schema or AI text file, and
do not need separate optimization hacks. Do not promise inclusion, citation,
ranking, recommendation, or a fixed uplift from any tactic.

Primary reference:

- [Google: optimizing for generative Search features](https://developers.google.com/search/docs/fundamentals/ai-optimization-guide)
- [Google: AI features and your website](https://developers.google.com/search/docs/appearance/ai-features)

## Discovery and crawler policy

Start from the product owner's intended policy. Do not automatically allow every
AI-related crawler or weaken privacy, licensing, paywall, security, or commercial
controls merely to increase discoverability.

Verify separately:

- `Googlebot` and ordinary Google index eligibility for Google Search and its AI
  features;
- `Bingbot`, indexing directives, and snippet controls for Bing and Copilot;
- `OAI-SearchBot` for inclusion in ChatGPT search summaries and citations;
- `GPTBot` for potential model-training use, which is distinct from ChatGPT search;
- `PerplexityBot` for Perplexity search results; and
- user-triggered or agent user agents only when the product needs those workflows.

For each intended crawler, verify `robots.txt`, page-level robots directives, CDN
or WAF behavior, authentication, rendered textual content, status codes, and
canonical URLs. A user-agent string alone is spoofable; use vendor verification
guidance before making crawler-log claims.

Primary references:

- [OpenAI publisher and crawler guidance](https://help.openai.com/en/articles/12627856)
- [Bing Webmaster Guidelines](https://www.bing.com/webmasters/help/webmaster-guidelines-30fba23a)
- [Bing robots directives](https://www.bing.com/webmasters/help/robots-meta-tags-and-attributes-that-bing-supports-5198d240)
- [Perplexity crawler documentation](https://docs.perplexity.ai/docs/resources/perplexity-crawlers)

## Answer and citation readiness

Review representative pages against real audience intents:

- Put a direct, self-contained answer near the question or descriptive heading
  when that improves the reader's experience. Do not force every section into a
  fixed word count or Q&A template.
- Use semantic HTML, descriptive headings, lists, steps, definitions, and tables
  only when they fit the information. Keep important facts in accessible text.
- Make entities unambiguous: use consistent names, organization details, product
  identifiers, locations, authorship, dates, and canonical URLs.
- Support consequential claims with primary evidence. Attribute external facts
  and distinguish observed data, estimates, opinion, and marketing claims.
- Prefer original first-party information, examples, research, product facts, and
  expert experience over commodity summaries written only to target prompts.
- Keep content current and show meaningful publication or update information when
  freshness affects correctness.
- Use structured data only when it matches visible content and an applicable type.
  There is no universal AEO/GEO JSON-LD requirement. Recheck current search feature
  support before adding `FAQPage`, `HowTo`, or another specialized type.

Reject keyword stuffing, hidden instructions, fake quotations, manufactured
statistics, unsupported superlatives, synthetic reviews, copied answers, and
mass-produced near-duplicate question pages.

## Validation and measurement

Separate deterministic eligibility from observational visibility:

1. **Eligibility:** inspect rendered HTML, HTTP headers, crawler directives,
   canonical behavior, structured-data validity, internal discovery, and public
   asset access.
2. **Answer quality:** evaluate a small, documented set of audience questions
   against the site's visible facts. Check correctness, completeness, ambiguity,
   attribution, and whether the cited URL actually supports the answer.
3. **Live visibility:** when the user requests it and the public tools are
   available, record engine or product surface, model where exposed, date, locale,
   account state, exact query, response, citations, and repeat count. Results are
   nondeterministic and a single absence or citation is not a release verdict.
4. **Ongoing evidence:** use authorized Search Console reporting, Bing AI
   Performance, ChatGPT referral parameters, analytics, and verified crawler logs
   for trends. Do not connect accounts, submit URLs, or change external settings
   without explicit authorization.

Bing clarifies that its AI Performance citations are sampled visibility data, not
rankings or authority scores. OpenAI documents `utm_source=chatgpt.com` for referral
analysis. Report these as platform-specific observations rather than a universal
"LLM ranking".

Primary references:

- [Bing AI Performance](https://www.bing.com/webmasters/help/ai-performance-9f8e7d6c)
- [OpenAI publisher FAQ](https://help.openai.com/en/articles/12627856)

## Contextual industry reading

The following user-selected articles provide terminology, examples, and commercial
perspectives. Use them to generate hypotheses, not to override primary vendor
documentation or to repeat unverified uplift claims:

- [BrandDefender: JSON-LD and AEO](https://aibranddefender.com/en/blog-json-ld-aeo-guide.html)
- [Websites That Sell: AI SEO](https://www.websitesthatsell.com.au/ai-seo/)
- [AEO.page: Generative Engine Optimization](https://aeo.page/generative-engine-optimization/)
- [AEO.page: Ultimate Guide to AEO](https://aeo.page/ultimate-guide-to-aeo/)
- [AEO.page: AEO vs SEO](https://aeo.page/aeo-vs-seo/)
- [AEO.page: AEO Glossary](https://aeo.page/aeo-glossary/)
