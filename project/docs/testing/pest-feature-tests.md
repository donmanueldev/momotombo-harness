# 🎯 Pest feature tests

## 💡 Convention

- Each user goal gets one feature test through HTTP: authenticated as a realistic user, asserting the response (status, redirect, Inertia component and props) **and** the resulting state.
- Each endpoint also gets tests for the failure paths users hit: validation errors, unauthorized access (another team's record), and not found.
- Business rules with several cases get unit tests on the Action, with cases picked by boundaries rather than by quantity.
- Build data with model factories and states. Never depend on seeded data or on test order.
- A new test must fail without the change it covers.

## 🏆 Benefits

- Tests prove behavior users rely on, so refactors stay safe.
- Authorization holes are caught before review.
- The suite stays fast and deterministic.

## 👀 Examples

### ✅ Good: goal, failure path, and authorization

```php
use Inertia\Testing\AssertableInertia as Assert;

use function Pest\Laravel\actingAs;

it('lets a member create a project', function () {
    $member = User::factory()->withTeam()->create();

    actingAs($member)->post('/projects', ['name' => 'Apollo'])
        ->assertRedirect();

    expect($member->currentTeam->projects()->where('name', 'Apollo')->exists())->toBeTrue();
});

it('rejects a duplicate name within the team', function () {
    $member = User::factory()->withTeam()->create();
    Project::factory()->for($member->currentTeam)->create(['name' => 'Apollo']);

    actingAs($member)->post('/projects', ['name' => 'Apollo'])
        ->assertSessionHasErrors('name');
});

it('hides other teams projects', function () {
    $project = Project::factory()->create();

    actingAs(User::factory()->withTeam()->create())
        ->get("/projects/{$project->id}")
        ->assertNotFound();
});

it('lists the team projects', function () {
    $member = User::factory()->withTeam()->create();
    Project::factory()->count(3)->for($member->currentTeam)->create();

    actingAs($member)->get('/projects')
        ->assertInertia(fn (Assert $page) => $page->component('projects/index')->has('projects', 3));
});
```

### ❌ Bad: asserts only the status code

```php
it('works', function () {
    $this->post('/projects', ['name' => 'x'])->assertStatus(302);
});
```

## 🧐 Real world examples

- <!-- Link a feature test from this project once it exists -->

## 🔗 Related agreements

- [Thin controllers and Actions](../backend/thin-controllers-and-actions.md)
