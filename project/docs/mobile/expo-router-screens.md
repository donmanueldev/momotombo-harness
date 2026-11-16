# 🎯 Expo Router screens

## 💡 Convention

- Files in `app/` are routes. A route file only composes a screen: read params, call one feature hook, and render. Logic, data fetching, and mutations live in feature modules (for example `src/features/<feature>/`).
- Every screen renders all its states: loading (a skeleton for content, a spinner only for short actions), empty (with the next action), error (a message plus retry), and success.
- Navigation params are typed, and only IDs travel in params. Load the record from the ID, never pass whole objects.
- Use native controls and platform conventions: safe areas, pull-to-refresh on lists, touch targets of at least 44pt, and keyboard avoidance on forms.
- Use the Expo plugin skills (`expo-router`, `expo-native-ui`, `expo-data-fetching`) for routing and UI details.

## 🏆 Benefits

- Screens stay small and testable, and features can move between routes freely.
- Users never see a frozen or blank screen.
- Deep links work, because every screen can load from its ID alone.

## 👀 Examples

### ✅ Good: thin route, feature hook, every state handled

```tsx
// app/projects/[id].tsx
export default function ProjectScreen() {
    const { id } = useLocalSearchParams<{ id: string }>();
    const { project, isLoading, error, refetch } = useProject(id);

    if (isLoading) return <ProjectSkeleton />;
    if (error) return <ErrorState message="Couldn't load the project." onRetry={refetch} />;
    if (!project) return <EmptyState title="Project not found" />;

    return <ProjectDetails project={project} />;
}
```

### ❌ Bad: fetching in the route, whole object in params, no states

```tsx
// app/projects/[id].tsx
export default function ProjectScreen() {
    const { project } = useLocalSearchParams(); // JSON-serialized object in the URL
    const [tasks, setTasks] = useState([]);
    useEffect(() => { fetch(`${API}/projects/${JSON.parse(project).id}/tasks`).then(r => r.json()).then(setTasks); }, []);
    return <FlatList data={tasks} renderItem={({ item }) => <Text>{item.title}</Text>} />;
}
```

## 🧐 Real world examples

- <!-- Link a screen from this project once it exists -->

## 🔗 Related agreements

- [Secure storage and API access](secure-storage-and-api.md)
- [Server and client state](server-and-client-state.md)
