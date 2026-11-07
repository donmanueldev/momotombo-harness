---
name: pagespeed-optimization
description: Measure and improve website loading, responsiveness, and visual stability using PageSpeed Insights, Lighthouse, CrUX, and Core Web Vitals. Use for an explicit performance audit, launch optimization, or measured web-performance regression; do not activate for ordinary UI work without a performance objective.
version: 1.1.0
status: stable
---

# PageSpeed Optimization

Improve user-visible web performance from repeatable evidence. Keep field data,
laboratory diagnostics, source/build proof, and deployed verification separate.
Do not promise rankings or treat a perfect Lighthouse score as the product goal.

## Establish a reproducible baseline

Before changing code:

1. Read repository instructions and identify representative URLs, the production
   build and serve path, existing performance tooling, and relevant device classes.
2. Record the exact URL, revision, environment, Lighthouse or PageSpeed version,
   mobile or desktop strategy, cache state, and major test conditions.
3. Capture mobile and desktop separately. Run five equivalent laboratory passes
   for each relevant configuration and use the median rather than the best run.
4. Preserve the raw or exported results outside tracked source unless the project
   already has an approved location for performance artifacts.

For a source-only task, record that field or deployed measurements remain pending.
Do not imply that a build proves real-user performance.

## Use the bundled automation

Detect the project's production adapter first:

```bash
<skill-path>/bin/detect-web-adapter --root <project-root> --format markdown
```

Read [Framework adapters](references/framework-adapters.md) for React/Vite,
Create React App, Next.js, Astro, and static HTML constraints. Verify the detected
commands against repository instructions before running them.

After starting the production build on a stable local or deployed URL, run:

```bash
<skill-path>/bin/run-lighthouse \
  --url http://127.0.0.1:4173/ \
  --profiles both \
  --runs 5 \
  --project-root <project-root> \
  --budget <budget.json> \
  --output-dir <artifact-directory>
```

The runner uses a project-local or explicitly supplied Lighthouse binary. It does
not download or install Lighthouse. It preserves each raw run, calculates medians,
writes JSON and Markdown reports, optionally compares a prior report with
`--baseline`, and exits nonzero when a configured budget fails. Copy and adapt
[`performance-budget.example.json`](assets/performance-budget.example.json) in the
target project when CI enforcement is requested.

## Separate field and laboratory evidence

- Prefer CrUX field data when it has sufficient coverage. It represents the
  previous 28 days at the 75th percentile and may be unavailable or fall back
  from a URL to origin-level data.
- Use Lighthouse laboratory data to diagnose reproducible problems and compare
  controlled revisions. A synthetic run does not represent every user's device,
  network, geography, cache, or interaction sequence.
- Evaluate current Core Web Vitals as LCP, INP, and CLS. Lighthouse does not
  measure INP directly; treat TBT only as a diagnostic proxy.

Read [Measurement and interpretation](references/measurement.md) before setting
thresholds, interpreting PageSpeed output, or comparing baselines.

## Optimize from causes

Prioritize changes that explain a measured bottleneck:

- Reduce render-blocking work and critical request chains when they delay LCP.
- Make the actual LCP resource discoverable and appropriately prioritized. Do not
  lazy-load it; reserve preload and `fetchpriority` for proven critical resources.
- Reduce long main-thread tasks, excessive JavaScript, and expensive interactions
  when responsiveness evidence points there.
- Reserve space and stabilize late content for layout-shift problems.
- Right-size and encode images according to content, fidelity, transparency, and
  compatibility. Do not impose WebP or AVIF when another format is more suitable.
- Improve caching, compression, fonts, third-party scripts, or server latency only
  when the trace or request evidence supports the change.

Preserve product behavior, accessibility, visual fidelity, analytics correctness,
and required third-party functionality. Do not install tooling, remove features,
or alter public behavior merely to raise a score without the user's authorization.

## Re-measure and report

Build the production configuration, repeat the same five-run matrix, and compare
medians. If conditions changed, label the comparison invalid or directional.

Report:

- tested URLs, revision, environment, device strategy, tool version, and run count;
- field and laboratory evidence in separate sections;
- baseline and after medians for the metrics relevant to each change;
- the causal finding, implementation, and observed effect;
- regressions or tradeoffs in behavior, accessibility, fidelity, or bundle cost;
- remaining opportunities ordered by impact and confidence; and
- validation that still requires a deployed URL or enough real-user traffic.

A 90–100 Lighthouse performance score is already categorized as good. Prefer
stable user outcomes over chasing 100 through brittle or misleading changes.
