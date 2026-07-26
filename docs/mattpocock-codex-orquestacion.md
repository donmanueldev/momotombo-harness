# Orquestación en Codex (estilo Matt)

Objetivo: menos ruido, más decisión técnica y cierre más limpio.

## 1) Estado actual del agente

- Plugins activos: `build-web-apps@openai-curated`, `expo@openai-curated`, `linear@openai-curated`, `vercel-plugin@plugins-cli`.
- Plugins desactivados: `.net` y `supabase`.
- Skills removidos del repositorio: `bun`, `electron-wrapper`, `react-doctor`, `to-prd`, `to-issues`.
- Skills actualizados desde Matt: `to-spec`, `to-tickets`, `implement`, `code-review`, `wayfinder`, `prototype`, `research`, `handoff`, `teach`, `writing-great-skills`, `grilling`, `resolving-merge-conflicts`.

## 2) Entrada única

Arranca siempre con `ask-matt`.

- `ask-matt` elige la ruta adecuada y evita invocar un flujo incorrecto.

## 3) Matriz de enrutamiento

- `setup-matt-pocock-skills`: bootstrap de tracker, labels y docs base.
- `to-spec`: convertir contexto en spec público y accionable.
- `to-tickets`: convertir spec en tickets de frontera (`ready-for-agent`) con dependencias.
- `implement`: ejecutar tickets con TDD y restricciones de seam.
- `code-review`: cerrar con revisión de estándar + cumplimiento de spec.
- `wayfinder`: planear esfuerzos grandes como mapa de decisiones.
- `planner`: definir alcance, riesgos y orden cuando hay ambigüedad.
- `grill-with-docs` / `grill-me`: clarificar decisiones de producto/arquitectura.
- `research`: recopilar evidencia externa cuando la decisión depende de terceros.
- `prototype`: reducir incertidumbre de comportamiento con artefacto de baja fidelidad.
- `resolving-merge-conflicts`: resolver conflictos de merge/rebase por intención.
- `triage`: ordenar entradas nuevas o regresiones.
- `diagnosing-bugs`: reproducir y depurar cambios rotos.
- `improve-codebase-architecture`: fortalecer estructura y modularidad.
- `domain-modeling`: estabilizar vocabulario y límites de dominio.

## 4) Composición por dominio

- `web`: UI, SEO, rendimiento frontend, experiencia producto.
- `backend`: contratos, flujo de negocio, APIs.
- `data`: modelo de datos, consistencia, migraciones/persistencia.
- `mobile`: UX y flujos móviles.
- `reviewer`: control de regresiones, seguridad y riesgo técnico al cierre.

## 5) Secuencia recomendada

1. `ask-matt`
2. `planner` cuando hay caminos alternos relevantes
3. `to-spec` para congelar intención
4. `to-tickets` para obtener unidades de ejecución
5. `implement` + `/tdd`
6. `code-review`
7. `reviewer` antes de marcar listo
8. Si cambia el alcance o aparece un bloqueo mayor, vuelve a `planner`

## 6) Plantilla de sesión

- Objetivo:
- Riesgo técnico principal:
- Plan:
- Agentes usados y entregables:
- Criterio de éxito:
- Riesgo residual y siguiente paso:

## 7) Mantenimiento

1. Revisa cambios de skills semanalmente.
2. Elimina skills fuera de tu stack.
3. Ejecuta `./sync.sh validate` y `./sync.sh status`.
4. Sincroniza nombres nuevos de Matt y mantén esta hoja como índice operativo.
