# 🎯 Form Requests and Policies

## 💡 Convention

Every write endpoint validates input in a Form Request and authorizes it with a Policy. The Form Request's `authorize()` delegates to the Policy. Controllers and Actions use only `$request->validated()`, never `$request->all()` or `$request->input()` for persisted data. Route-model-bound records are authorized against the current user, so users can't reach other teams' records by changing an ID; the Policy answers with `Response::denyAsNotFound()` so it doesn't reveal that the record exists. Endpoints with no input (a delete, a toggle) still authorize, with `->can('delete', 'project')` on the route or `Gate::authorize()`. Models declare their mass-assignable columns (`#[Fillable([...])]` in Laravel 13) and are never unguarded.

## 🏆 Benefits

- Validation and authorization live in one predictable place per endpoint.
- Closes mass-assignment and IDOR holes (`/projects/42` belonging to another team).
- Validation errors flow back to Inertia forms automatically as `errors`.

## 👀 Examples

### ✅ Good: validated data and policy-based authorization

```php
final class UpdateProjectRequest extends FormRequest
{
    public function authorize(): bool
    {
        return $this->user()->can('update', $this->route('project'));
    }

    public function rules(): array
    {
        return [
            'name' => ['required', 'string', 'max:120',
                Rule::unique('projects')->where('team_id', $this->route('project')->team_id)
                    ->ignore($this->route('project'))],
        ];
    }
}

final class ProjectPolicy
{
    public function update(User $user, Project $project): bool
    {
        return $user->belongsToTeam($project->team);
    }
}
```

### ❌ Bad: no authorization and raw input

```php
public function update(Request $request, Project $project)
{
    $project->update($request->all()); // any user, any field, any team
}
```

## 🧐 Real world examples

- <!-- Link a Form Request and Policy from this project once they exist -->

## 🔗 Related agreements

- [Thin controllers and Actions](thin-controllers-and-actions.md)
