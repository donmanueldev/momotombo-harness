# 🎯 Secure storage and API access

## 💡 Convention

- Store tokens and secrets only in `expo-secure-store` (Keychain / Keystore), never in AsyncStorage, MMKV without encryption, or app state that persists to disk.
- Never ship secrets in the bundle. `EXPO_PUBLIC_*` variables are public by definition, so use them only for public config such as the API base URL.
- All HTTP calls go through one API client that adds the auth header, handles `401` by clearing the session and routing to sign-in, and turns errors into typed results the UI can render.
- Validate API responses at the boundary (for example with zod) before they reach screens.

## 🏆 Benefits

- A stolen or rooted device doesn't leak plain-text tokens.
- Session expiry behaves the same everywhere in the app.
- A backend change that breaks the response shape fails loudly in one place, not as a crash deep in a screen.

## 👀 Examples

### ✅ Good: secure storage and one client

```ts
import * as SecureStore from 'expo-secure-store';

export async function saveSession(token: string) {
    await SecureStore.setItemAsync('session-token', token);
}

export async function api<T>(path: string, schema: z.ZodType<T>, init?: RequestInit): Promise<T> {
    const token = await SecureStore.getItemAsync('session-token');
    const response = await fetch(`${process.env.EXPO_PUBLIC_API_URL}${path}`, {
        ...init,
        headers: { Accept: 'application/json', ...(token && { Authorization: `Bearer ${token}` }), ...init?.headers },
    });
    if (response.status === 401) {
        await SecureStore.deleteItemAsync('session-token');
        router.replace('/sign-in');
    }
    if (!response.ok) throw new ApiError(response.status, await response.text());

    return schema.parse(await response.json());
}
```

### ❌ Bad: plain-text token and a secret in the bundle

```ts
await AsyncStorage.setItem('token', token);
const STRIPE_SECRET = process.env.EXPO_PUBLIC_STRIPE_SECRET; // shipped to every device
```

## 🧐 Real world examples

- <!-- Link the API client from this project once it exists -->

## 🔗 Related agreements

- [Expo Router screens](expo-router-screens.md)
- [Server and client state](server-and-client-state.md)
