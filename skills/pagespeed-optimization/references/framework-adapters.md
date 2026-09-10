# Framework Adapters

Use `../scripts/detect_web_adapter.py --root <project>` to identify a starting
build and serve contract. Treat its output as detected evidence, then verify the
project's own instructions, scripts, environment, routes, rewrites, and headers.

## Shared rule

Audit a production build through an HTTP URL. Do not report measurements from a
framework development server as production evidence. Start the server separately,
wait for an explicit health response, run the SEO and Lighthouse automation, and
stop only the process started for the audit.

Generated reports and raw Lighthouse runs should normally go to a temporary or
ignored artifact directory. Do not add them to source control unless the project
defines a versioned performance-baseline policy.

## React with Vite

- Detect `react` and `vite` in `package.json`.
- Use the repository's build script, then its preview script with an explicit
  loopback host and port.
- Default output is commonly `dist`, but confirm `build.outDir` in Vite config.
- A static preview cannot prove CDN caching, compression, redirects, CSP, or edge
  behavior; validate those on the deployed environment separately.

## React with Create React App

- Build with the repository's build script and audit the generated `build` output.
- Python's static server is a local fallback only. It does not reproduce SPA
  rewrites, compression, cache headers, or hosting-platform behavior.
- Confirm that deep routes return the intended HTML rather than a local-only 404.

## Next.js

- Use the production `build` and `start` scripts. Do not audit `next dev`.
- Test representative static, server-rendered, dynamic, localized, and error
  routes separately; one homepage result cannot characterize the whole app.
- Inspect rendered metadata and response headers because App Router metadata,
  route handlers, redirects, image optimization, fonts, caching, and middleware
  may differ by route and runtime.
- A locally started Node server does not prove Vercel or CDN behavior.

## Astro

- Use the production build and preview scripts with an explicit host and port.
- Check the configured output mode and adapter. Static `dist` output and an SSR
  Node, serverless, or edge adapter have different runtime and header behavior.
- Audit representative generated and dynamic routes plus localized variants.

## Static HTML

- Serve the intended publish directory over HTTP; do not audit files through
  `file://` because URL resolution and browser security behavior differ.
- Verify directory indexes, relative assets, canonical origins, redirects, error
  pages, compression, and cache headers on the real host separately.

## CI integration

Pin Lighthouse as a project devDependency and invoke
`scripts/run_lighthouse.py --lighthouse-bin node_modules/.bin/lighthouse`.
Use a project-owned budget copied from
`../assets/performance-budget.example.json`. CI should retain JSON, Markdown, and
raw runs as artifacts and fail only on explicit budget violations or audit errors.

The bundled budget file is intentionally a small runner-specific contract, not
the native Lighthouse or Lighthouse CI `budgets.json` format. It accepts
`default`, `mobile`, and `desktop` objects; profile values override defaults.

| Metric | Unit | Typical rule |
| --- | --- | --- |
| `performanceScore` | `0` to `1` | `min` |
| `firstContentfulPaintMs` | milliseconds | `max` |
| `largestContentfulPaintMs` | milliseconds | `max` |
| `speedIndexMs` | milliseconds | `max` |
| `totalBlockingTimeMs` | milliseconds | `max` |
| `cumulativeLayoutShift` | unitless | `max` |
| `totalByteWeightBytes` | bytes | `max` |

If the target project already uses Lighthouse CI, prefer its maintained collection
and assertion system instead of adding a parallel runner. Preserve the same rules:
pin tooling in the project, collect repeated equivalent runs, retain artifacts,
and separate laboratory results from field evidence.

- [Lighthouse CLI options](https://github.com/GoogleChrome/lighthouse/blob/main/readme.md#using-the-node-cli)
- [Lighthouse CI configuration](https://github.com/GoogleChrome/lighthouse-ci/blob/main/docs/configuration.md)
