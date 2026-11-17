# Kitchen: Laravel

Verified against the Larastan, Rector, and rector-laravel docs (November 2026). Check the installed versions' docs before changing this setup.

## Tools

| Tool | Catches | Package |
|---|---|---|
| Pint | formatting | `laravel/pint` (ships with Laravel) |
| Larastan | type errors, wrong Eloquent usage, dead branches | `larastan/larastan` |
| Rector + rector-laravel | legacy and deprecated patterns, dead code, missing types | `rector/rector`, `driftingly/rector-laravel` |
| Pest | behavior | `pestphp/pest` (ships with the starter kits) |

`harness init` adds `phpstan.neon.dist` (level 8) and `rector.php` (PHP and Laravel version sets from `composer.json`, dead code, code quality, type declarations, early return, Laravel code quality and collections) when they are missing.

## Setup

1. Ask the user, then install: `composer require --dev larastan/larastan rector/rector driftingly/rector-laravel`.
2. Add the scripts to `composer.json`:
   ```json
   "check": ["pint --test", "phpstan analyse --memory-limit=1G", "rector --dry-run", "pest --parallel"],
   "check:fast": ["pint --dirty --test", "pest --dirty"]
   ```
3. Existing code: run `vendor/bin/rector` once and review the diff as its own commit. Then run `vendor/bin/phpstan analyse --generate-baseline` and add `phpstan-baseline.neon` to the `includes` of `phpstan.neon.dist`, so new code meets level 8 while the backlog shrinks.
4. Name both commands in the project's `AGENTS.md` (`**Check:**` and `**Fast check:**`).
5. Prove it: write a method with a missing return type and a `compact()` call in a controller; `composer check` must fail. Revert it.

## What this enforces, and what stays with you

- Rector rewrites deprecated framework calls and legacy patterns, so the agent stops copying them.
- Larastan rejects untyped and wrongly typed code at level 8.
- Still on you, per `principles/authorize-at-the-boundary.md`: a Form Request for every input and a Policy for every protected action.
