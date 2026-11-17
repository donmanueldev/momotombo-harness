# 🎯 Thin controllers and Actions

## 💡 Convention

A controller method does three things: receive a validated Form Request, call **one** Action, and return a response (an Inertia page, a redirect, or an API Resource). Business logic lives in single-purpose Action classes in `app/Actions/`, each with one public `handle()` method. Actions receive plain data and models, never the `Request`. Events dispatched inside an Action's transaction implement `ShouldDispatchAfterCommit` (jobs use `->afterCommit()`), so queued listeners never run before the commit.

## 🏆 Benefits

- Business rules are reusable from controllers, jobs, commands, and tests.
- Unit tests target the Action directly, without HTTP.
- Controllers stay readable, and a reviewer sees the flow at a glance.

## 👀 Examples

### ✅ Good: validate, delegate, respond

```php
final class ProjectController extends Controller
{
    public function store(StoreProjectRequest $request, CreateProject $createProject): RedirectResponse
    {
        $project = $createProject->handle($request->user(), $request->validated());

        return to_route('projects.show', $project);
    }
}

final class CreateProject
{
    public function handle(User $owner, array $attributes): Project
    {
        return DB::transaction(function () use ($owner, $attributes) {
            $project = $owner->currentTeam->projects()->create($attributes);
            ProjectCreated::dispatch($project);

            return $project;
        });
    }
}
```

### ❌ Bad: validation, rules, and side effects inside the controller

```php
public function store(Request $request)
{
    $request->validate(['name' => 'required']);
    if (Project::where('name', $request->name)->exists()) {
        return back()->withErrors(['name' => 'Taken']);
    }
    $project = Project::create($request->all()); // mass-assigns anything the client sends
    Mail::to($request->user())->send(new ProjectCreatedMail($project));

    return redirect('/projects/'.$project->id);
}
```

## 🧐 Real world examples

- <!-- Link a controller and Action from this project once they exist -->

## 🔗 Related agreements

- [Form Requests and Policies](form-requests-and-policies.md)
- [Pest feature tests](../testing/pest-feature-tests.md)
