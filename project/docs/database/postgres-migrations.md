# 🎯 PostgreSQL migrations

## 💡 Convention

- Columns are `NOT NULL` unless the domain truly has an unknown value. Give new columns on existing tables a default, or backfill them in the same deploy.
- Use `text` for free text. Use `string(n)` only when a maximum length is a business rule, and enforce it in validation too.
- Every foreign key gets an explicit index (PostgreSQL does **not** index foreign key columns automatically) and an explicit `on delete` behavior.
- Uniqueness rules from the domain are database constraints, not just validation.
- Changes to large tables (backfills, new indexes) are planned so they don't lock writes, and they are called out in the PR. Index a large table with `->online()` (PostgreSQL `CREATE INDEX CONCURRENTLY`) in its own migration with `public $withinTransaction = false;`, because a concurrent index can't be built inside a transaction.

## 🏆 Benefits

- The database enforces integrity even when the code has a bug.
- Queries joining on foreign keys stay fast as tables grow.
- Deploys are predictable, and teammates' databases never drift.

## 👀 Examples

### ✅ Good: constraints and indexes declared explicitly

```php
Schema::create('projects', function (Blueprint $table) {
    $table->id();
    $table->foreignId('team_id')->index()->constrained()->cascadeOnDelete();
    $table->text('name');
    $table->text('description')->nullable(); // optional by design
    $table->timestamps();

    $table->unique(['team_id', 'name']);
});
```

### ❌ Bad: nullable by default, no index, uniqueness only in code

```php
Schema::create('projects', function (Blueprint $table) {
    $table->id();
    $table->unsignedBigInteger('team_id')->nullable();
    $table->string('name')->nullable();
    $table->timestamps();
});
```

## 🧐 Real world examples

- <!-- Link a migration from this project once it exists -->

## 🔗 Related agreements

- Laravel Boost skill `laravel-best-practices` (migrations, validation): the unique validation rule mirrors the constraint.
