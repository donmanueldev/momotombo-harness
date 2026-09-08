# Plantilla de solicitud para orquestación (bug, mejora, estado)

Esta plantilla estandariza cómo pedir trabajo para que el orquestador y los agentes especializados puedan enrutar y ejecutar de forma confiable.

## Comportamiento con entradas cortas

Si escribes algo ambiguo como:

```text
mejora el login
```

se tratará como `tipo: mejora` y el agente te pedirá estos datos mínimos para poder avanzar:

```text
tipo: mejora
objetivo: mejorar el login
contexto: repo=...
alcance:
  incluir: login y flujo de recuperación (si aplica)
  excluir: cambios en otras pantallas
criterios_de_exito:
  - login más rápido/robusto
```

Con eso, `specifier` puede producir una especificación verificable. Tras su aprobación explícita, `planner` prepara el plan y `orchestrator` enruta tareas verificables.

## Campos mínimos (siempre)

- `tipo`: `bug` | `mejora` | `estado`
- `objetivo`: qué resultado quieres
- `contexto`: repo, módulo, ticket, rama, entorno
- `alcance`: qué entra y qué no entra
- `criterios_de_exito`: comprobables y mesurables
- `restricciones`: deadlines, riesgo, límites de stack/servicios/entorno
- `evidencia`: links, logs, pruebas, pantallazos, comandos, errores
- `siguiente_paso`: qué se debería hacer con el output

## Plantilla para BUG

```text
tipo: bug
objetivo: corregir X
contexto: repo/modulo/version de entorno
sintoma:
reproduccion:
comportamiento_esperado:
comportamiento_actual:
impacto:
evidencia:
plazo:
criterios_de_exito:
  - se reproduce + se verifica en entorno Y
  - pruebas pasan en caso aplicable
  - regresión cubierta (o regresión explicada y bloqueada)
```

Ruta sugerida:

- planificación y reproducción del fallo
- implementación por el especialista de dominio
- `reviewer` si hay riesgo o cambio de alcance funcional

## Plantilla para MEJORA

```text
tipo: mejora
objetivo: añadir/completar X
contexto: repo/modulo
alcance:
  incluir:
  excluir:
impacto_usuario:
riesgos:
dependencias:
criterios_de_exito:
  - cambio observable
  - tests/validaciones solicitadas
  - rollback esperado
```

Ruta sugerida:

- `specifier` produce una spec con alcance, no-alcance, requisitos, aceptación y matriz de verificación
- aprobación explícita de la spec antes de planificar o implementar
- `planner` conserva IDs de requisito en el plan
- tareas separables con dependencias explícitas
- implementación por vertical slices
- revisión independiente antes de marcar listo

## Plantilla para ESTADO / ANALISIS

```text
tipo: estado
objetivo: diagnosticar estado actual de X
contexto: repo/modulo
pregunta_clave:
  - ¿qué está estable?
  - ¿qué está roto/riesgoso?
  - ¿qué sigue?
evidencia:
  - diff reciente, tests ejecutados, incidentes, issues
criterios_de_exito:
  - resumen accionable por severidad
  - próximos pasos con dueño/orden
```

Ruta sugerida:

- inspección de código, documentación y evidencia disponible
- clasificación por impacto si requiere seguimiento en tablero

## Reglas de salida esperadas

- `specifier`, el planificador y el orquestador deben responder con un único objeto JSON según:
- `docs/orchestration-output-contract.md`
- Antes de finalizar cada fase, registrar en:
  - `.codex/orchestration.jsonl`
- Comando recomendado:

```bash
bash scripts/log-orchestration-event.sh \
  --agent <specifier|planner|orchestrator|reviewer> \
  --goal "<objetivo>" \
  --status <ok|blocked|failed|in_progress> \
  --decision "<decision>" \
  --result "<resultado>" \
  --next-step "<siguiente>"
```
