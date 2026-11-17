# 🎯 Server and client state

## 💡 Convention

- **Server state** (anything the API owns) lives only in TanStack Query. Each resource gets one query hook in its feature module, with a stable query key, and mutations invalidate or update those keys.
- **Client state** (UI choices the server doesn't know about, such as filters, a draft, or onboarding seen) lives in Zustand, or in `useState` when one screen owns it. Never copy query data into a Zustand store or into `useState`.
- Wiring TanStack Query to the device (online and focus managers, offline persistence) follows the `expo-data-fetching` skill and the TanStack Query React Native docs for the installed version.
- A persisted query cache is plain text: keep personal data out of it (`dehydrateOptions.shouldDehydrateQuery`). Tokens follow [Secure storage and API access](secure-storage-and-api.md).
- Show the user when they are offline and when a mutation is waiting to send.

## 🏆 Benefits

- One source of truth per piece of data, so screens never show two different values.
- Caching, retries, refetching, and offline behavior come from the library instead of hand-written effects.
- Zustand stores stay small and only hold what the user chose.

## 👀 Examples

### ✅ Good: a query hook for server data, a store for a UI choice

```ts
export function useExpenses(month: string) {
    return useQuery({ queryKey: ['expenses', month], queryFn: () => api(`/expenses?month=${month}`, ExpenseList) });
}

export const useExpenseFilters = create<ExpenseFilters>()((set) => ({
    category: null,
    setCategory: (category) => set({ category }),
}));
```

### ❌ Bad: server data copied into a store and fetched in an effect

```ts
export const useExpenseStore = create((set) => ({
    expenses: [],
    loading: false,
    fetchExpenses: async (month) => {
        set({ loading: true });
        const expenses = await api(`/expenses?month=${month}`, ExpenseList);
        set({ expenses, loading: false }); // stale after any mutation, no retry, no offline handling
    },
}));
```

## 🧐 Real world examples

- <!-- Link a query hook and a store from this project once they exist -->

## 🔗 Related agreements

- [Expo Router screens](expo-router-screens.md)
- [Secure storage and API access](secure-storage-and-api.md)
- [Native config and permissions](native-config-and-permissions.md)
