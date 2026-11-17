# 🎯 Inertia forms and UI states

## 💡 Convention

- Inertia v3 with React 19. Pages receive typed props: type each page's props, `usePage<T>()`, and the shared data once in `InertiaConfig.sharedPageProps`. Never use `any`.
- Forms use Inertia's `<Form>` component, or `useForm<T>()` when the form's state must be controlled in code. Server validation errors show next to their field, the submit button is disabled while `processing` (true until `onFinish`), and input is kept on error.
- Every page handles its states: empty (with the next action to take), error, and success feedback: a redirect, or a toast sent with `Inertia::flash()` and read from `page.flash`.
- Page data comes from Inertia props and refreshes with partial reloads (`router.reload({ only: [...] })`). Use TanStack Query only for client-fetched data that isn't a page prop (live search, polling, infinite lists), and never copy props into its cache.
- Build UI from the project's shadcn/ui components and Tailwind tokens, not one-off styles. Every control has a label, and errors are announced (`aria-invalid`, `role="alert"`).
- Pages stay presentational. Derived data comes from the server in props, and reusable client logic goes in hooks.

## 🏆 Benefits

- Users always know what is happening and what to do next.
- Validation feedback is consistent across the app without extra code.
- Type errors catch prop mismatches between Laravel and React before runtime.

## 👀 Examples

### ✅ Good: typed props, field errors, pending state, empty state

```tsx
type Props = { projects: Array<{ id: number; name: string }> };

export default function ProjectsIndex({ projects }: Props) {
    const { data, setData, post, processing, errors, reset } = useForm({ name: '' });

    function submit(e: React.FormEvent) {
        e.preventDefault();
        post('/projects', { onSuccess: () => reset() });
    }

    return (
        <>
            <form onSubmit={submit} className="flex flex-col gap-2">
                <Label htmlFor="name">Project name</Label>
                <Input id="name" value={data.name} aria-invalid={!!errors.name}
                       onChange={(e) => setData('name', e.target.value)} />
                {errors.name && <p role="alert" className="text-sm text-destructive">{errors.name}</p>}
                <Button type="submit" disabled={processing}>
                    {processing ? 'Creating…' : 'Create project'}
                </Button>
            </form>

            {projects.length === 0
                ? <EmptyState title="No projects yet" description="Create your first project to get started." />
                : <ProjectList projects={projects} />}
        </>
    );
}
```

### ❌ Bad: untyped, silent errors, double submits, blank screen when empty

```tsx
export default function ProjectsIndex(props: any) {
    const [name, setName] = useState('');
    return (
        <div>
            <input onChange={(e) => setName(e.target.value)} />
            <button onClick={() => router.post('/projects', { name })}>Submit</button>
            {props.projects.map((p: any) => <div>{p.name}</div>)}
        </div>
    );
}
```

## 🧐 Real world examples

- <!-- Link a page from this project once it exists -->

## 🔗 Related agreements

- Laravel Boost skills `inertia-react-development` (forms, props) and `laravel-best-practices` (validation, the source of `errors`).
