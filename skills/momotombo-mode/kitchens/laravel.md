# Kitchen: Laravel

Verified against the Laravel 13, Larastan, Rector, rector-laravel, and Pest docs (November 2026). Check the installed versions' docs before changing this setup.

## Tools

| Tool | Catches | Package |
|---|---|---|
| Pint | formatting | `laravel/pint` (ships with Laravel) |
| Larastan | type errors, wrong Eloquent usage, dead branches | `larastan/larastan` |
| Rector + rector-laravel | legacy and deprecated patterns, dead code, missing types | `rector/rector`, `driftingly/rector-laravel` |
| Pest | behavior | `pestphp/pest` (ships with the starter kits; Pest 5 needs PHP 8.4+, so on PHP 8.3 stay on Pest 4) |

`harness init` adds `phpstan.neon.dist` (level 8) and `rector.php` (PHP and Laravel version sets from `composer.json`, dead code, code quality (including early return), type declarations, Laravel code quality and collections) when they are missing.

## Setup

1. Laravel Boost first: if `boost.json` is missing, ask the user, then `composer require laravel/boost --dev` and `php artisan boost:install` (guidelines, skills, MCP for Claude Code and Codex). On an existing app, run the `infer-conventions` skill to record its conventions as `.ai/rules`. Boost owns how to write Laravel; this kitchen only adds what rejects bad code.
2. Ask the user, then install: `composer require --dev larastan/larastan rector/rector driftingly/rector-laravel`.
3. Add the scripts to `composer.json`:
   ```json
   "check": ["pint --test", "phpstan analyse --memory-limit=1G", "rector --dry-run", "pest --parallel"],
   "check:fast": ["pint --dirty --test", "pest --dirty"]
   ```
4. Existing code: run `vendor/bin/rector` once and review the diff as its own commit. Then, with the user's approval, run `vendor/bin/phpstan analyse --generate-baseline` and add `phpstan-baseline.neon` to the `includes` of `phpstan.neon.dist`, so new code meets level 8 while the backlog shrinks.
5. Name both commands in the project's `AGENTS.md` (`**Check:**` and `**Fast check:**`).
6. Prove it: write a method with a missing return type and a `compact()` call in a controller; `composer check` must fail. Revert it.

## What this enforces, and what stays with you

- Rector rewrites deprecated framework calls and legacy patterns, so the agent stops copying them.
- Larastan rejects untyped and wrongly typed code at level 8.
- Still on you, per `principles/authorize-at-the-boundary.md`: a Form Request for every input and a Policy for every protected action.
