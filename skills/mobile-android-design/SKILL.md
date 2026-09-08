---
name: mobile-android-design
description: Design and implement accessible, adaptive Android interfaces with Material 3 and Jetpack Compose. Use for Android UI, navigation, theming, gestures, and screen-size adaptation.
metadata:
  source: https://github.com/wshobson/agents/tree/main/plugins/ui-design/skills/mobile-android-design
  local_adaptation: agent-config
---

# Android Mobile Design

Use Material 3 and the application's existing design system to deliver native Android UI that is legible, accessible, and appropriate for the devices the app supports.

## Workflow

1. Inspect the current theme, navigation, resources, supported API levels, and affected screen before choosing components or changing layout structure.
2. Reuse existing composables, tokens, typography, and navigation contracts. Introduce a new design-system primitive only when repeated behavior cannot be expressed with the current surface.
3. Use `MaterialTheme.colorScheme` and semantic roles; do not hard-code colors merely to reproduce a screenshot. Support dynamic color only when it matches the product's existing theme policy.
4. Make layouts adaptive for the app's supported window sizes and input methods. Use lazy containers for large collections, keep scroll containers non-nested in the same axis, and preserve state across recreation when the product requires it.
5. Give interactive controls accessible labels, at least 48dp touch targets where applicable, visible focus/selection state, and sufficient contrast.

## Validation

- Verify previews or screenshot tests where the project already uses them, then build the affected Android variant.
- Exercise navigation, configuration change, font scaling, TalkBack-relevant labels, and the target form factor when they are in scope.
- Report host tests, build success, emulator validation, and physical-device evidence separately. A successful build is not proof of rendering, gestures, or accessibility.

## Boundaries

- Prefer the smallest causal UI change; do not replace the app architecture or theme as part of a screen task.
- Follow current official Android and Material documentation when this skill conflicts with project-local guidance.
