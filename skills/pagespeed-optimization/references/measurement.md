# Measurement and Interpretation

Use this reference when collecting or interpreting PageSpeed Insights, CrUX, or
Lighthouse evidence.

## Core Web Vitals

Field Core Web Vitals are assessed at the 75th percentile of page loads:

| Metric | Good | Needs improvement | Poor |
| --- | ---: | ---: | ---: |
| Largest Contentful Paint (LCP) | `<= 2.5 s` | `> 2.5 s` and `<= 4 s` | `> 4 s` |
| Interaction to Next Paint (INP) | `<= 200 ms` | `> 200 ms` and `<= 500 ms` | `> 500 ms` |
| Cumulative Layout Shift (CLS) | `<= 0.1` | `> 0.1` and `<= 0.25` | `> 0.25` |

Use the latest definitions from [Web Vitals](https://web.dev/articles/vitals)
when tooling or documentation changes.

## Field data

PageSpeed Insights can expose Chrome User Experience Report data for eligible
public URLs or origins. It aggregates the previous 28 days and requires enough
traffic, so a new, private, low-traffic, or recently changed page may have no URL
data or only origin data. Field evidence is historical and environment-diverse;
it cannot immediately prove a new code revision.

Record whether data is URL-level, origin-level, or unavailable. Never silently
substitute origin data for a page-specific claim.

References:

- [About PageSpeed Insights](https://developers.google.com/speed/docs/insights/v5/about)
- [Chrome UX Report methodology](https://developer.chrome.com/docs/crux/methodology)

## Laboratory data

Lighthouse is a controlled diagnostic. Its scores vary with hardware, network,
background load, page nondeterminism, extensions, and tool versions.

- Compare equivalent production builds and configurations.
- Run five times and report the median. Keep individual results available when
  they reveal instability rather than hiding it.
- Lighthouse cannot observe real-user INP; TBT may help diagnose main-thread
  blocking but is not an INP replacement.
- Do not freeze audit names, weights, or recommendations into policy. Lighthouse
  and PageSpeed scoring models change.
- A performance score of 90–100 is good. The score is a weighted laboratory
  summary, not a search-ranking or release guarantee.

References:

- [Lighthouse variability](https://github.com/GoogleChrome/lighthouse/blob/main/docs/variability.md)
- [Lighthouse performance scoring](https://developer.chrome.com/docs/lighthouse/performance/performance-scoring)
- [PageSpeed Insights release notes](https://developers.google.com/speed/docs/insights/release_notes)
- [Why field and lab data differ](https://web.dev/articles/lab-and-field-data-differences)

## Comparison record

For each test set, retain or report:

- URL and git revision;
- production build and server mode;
- date, tool name, and version;
- mobile or desktop strategy and throttling configuration;
- warm or cold cache state;
- five individual results and their median; and
- known differences between baseline and after runs.

Reject a comparison as causal proof when the URL, content, device strategy, tool
version, or runtime environment changed materially between samples.
