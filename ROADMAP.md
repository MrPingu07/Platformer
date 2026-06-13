# Roadmap

## En progreso
_nada actualmente_

---

## Pendiente

### Enemigos
- **Gunner** — Variante del Runner. Mantiene distancia del jugador, dispara ráfagas tipo semiauto. Reutiliza `bullet.c` y `Hittable`.
- **Mosquito** — Patrullero volador. Ignora geometría del mundo. Al detectar al jugador carga en patrón errático. Sin gravedad.

### Escenas
- **Scene Manager** — Módulo centralizado `scene_manager.c/.h`. Registro de escenas por ID, transición via `scene_manager_goto(int id)`. Las escenas no se conocen entre sí. Sin pila por ahora.
- **Menú principal** — Primera escena no-gameplay. Prerequisito: Scene Manager.

### Metadata de nivel
- **Cabecera en `.txt`** — Líneas `clave=valor` antes del grid, separadas por `---`. Parseadas en `LevelData` como campos tipados.
- **Campos previstos:**
  - `next=example-level2.txt` — Nivel destino al completar
  - `music=theme1` — ID de música (futuro)
  - `tileset=example` — Pack de tiles a usar para este nivel
- **Exit tile** — Token nuevo en el grid que representa el punto de salida. Activo solo cuando las condiciones de victoria están satisfechas.

### Sistema de Tilesets
El designer coloca los tiles manualmente en el `.txt`. El engine no infiere ni calcula vecinos: lo que está en el grid es lo que se renderiza. Sin magia.

**Tokens de geometría sólida:**
```
↖ ↑ ↗
←  #  →      9 variantes direccionales + 1 aislado
↙ ↓ ↘
=            Tile aislado — se ve bien sin vecinos
```

**Formato del pack:** un spritesheet PNG por tileset, 10 tiles en fila en orden fijo:
```
↖ ↑ ↗ ← # → ↙ ↓ ↘ =
```
Cada tile ocupa `TILE_SCREEN_SIZE x TILE_SCREEN_SIZE` px. Un solo `LoadTexture()` por nivel, `DrawTextureRec()` por tile en render.

**Ubicación:** `assets/tilesets/<nombre>/tileset.png`

**Responsabilidades:**
- `level_loader.c` lee `tileset=example` de la cabecera y lo guarda como string en `LevelData`. Ciego a Raylib, ciego a assets.
- `game.c` recibe el string, llama a `tileset_load("assets/tilesets/example")` al iniciar la escena y `tileset_unload()` al salir.
- El renderer de plataformas en `game.c` mapea cada token a su índice en el spritesheet y llama `DrawTextureRec()`.

**Prerequisito:** Cabecera de metadata en `.txt`.

### Plataformas especiales

**Plataforma móvil**
- Token `M` en el grid. Posición de spawn únicamente.
- Path, velocidad, aceleración, deceleración y sprite definidos en la cabecera por índice de aparición:
  ```
  moving:0=pathStartX,pathStartY,pathEndX,pathEndY,speed,accel,decel,spriteIndex
  moving:1=...
  ```
- `spriteIndex` apunta a un tile del spritesheet del tileset activo. Sin tokens adicionales en el grid.
- `Struct: MovingPlatform { rect, velocity, pathStart, pathEnd, t, speed, accel, decel, spriteIndex }`
- El player resuelve colisión contra ella con `resolve_rect_collision`. Rider logic (player se mueve con la plataforma) es el único añadido no trivial.

**Plataforma rompible**
- Token `X` en el grid.
- `Struct: CrumblingPlatform { rect, state, timer }`
- Estados: `IDLE -> SHAKING -> BROKEN`. Respawn opcional por nivel.
- Colisión desactivada en estado `BROKEN`.

**Prerequisito:** Cabecera de metadata en `.txt`.

### Sistema de condiciones de victoria
- Definidas en la metadata del nivel, evaluadas cada frame por `game.c`.
- Soporte para condiciones AND y OR.
- **Condiciones previstas:**
  - `win=kill_all` — Eliminar todos los enemigos del nivel
  - `win=collect_keys:3` — Recoger N llaves
  - `win=break_all_boxes` — Romper todas las cajas
  - `win=reach_exit` — Llegar al tile de salida (A a B simple)
  - `win=kill_boss` — Eliminar al jefe del nivel

---

## Completado
- Arquitectura base: scene system, módulos ciegos, zero heap
- `TILE_SIZE` centralizado en `defines.h` — todas las dimensiones, velocidades y fuerzas son ratios de `TILE_SIZE`
- `TILE_SCREEN_SIZE` separado de `TILE_SIZE` — zoom de cámara independiente de la escala de simulación
- Culling rectangular en world-space: render (viewport exacto) y lógica (3x viewport)
- Player: movimiento, salto, agacharse, sistema de apuntado direccional
- Runner: patrulla, detección, memoria, aggro, chance de drops al morir
- Sistema de armas polimórfico: Semiauto, Shotgun, Full-Auto, Lanzallamas
- Pool de balas, sistema de drops con física
- Cámara con deadzone en screen space, lerp, clamp de nivel
- Level loader data-driven desde `.txt`
- Arrays escalados a 2048
- Kill volume para player, runners y drops
- Flag `-DDEBUG` en compilación para HUD y overlays de debug
- Player respawnea con loadout inicial al morir
- Drop chance dinámico por kills, topa en 80%
