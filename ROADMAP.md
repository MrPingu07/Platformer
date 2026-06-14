# Roadmap

## En progreso
_nada actualmente_

---

## Pendiente

### Enemigos
- **Gunner** — Variante del Runner. Mantiene distancia del jugador, dispara ráfagas tipo semiauto. Reutiliza `bullet.c` y `Hittable`.
- **Mosquito** — Patrullero volador. Ignora geometría del mundo. Al detectar al jugador carga en patrón errático. Sin gravedad.

### Escenas
- **Scene Manager** — Implementado en forma primitiva (`scene_manager_set`, `update`, `render`). Sin registro por ID ni pila. Expandir cuando haya más escenas que lo justifiquen.
- **Menú principal** — Implementado: Main (Play / Settings) → Settings (Resolution / Return) → Resolution (lista dummy). Pendiente: conectar resoluciones reales con `SetWindowSize`.

### Metadata de nivel
- **Exit tile** — Token `E` en el grid parsea posición y destino. Pendiente: activación condicional (victoria requerida antes de poder usarlo).

### Sistema de Tilesets
El designer coloca los tiles manualmente en el `.txt`. El engine no infiere vecinos.

Tokens de geometría sólida (numpad layout):
```
7 8 9
4 5 6
1 2 3
=
```

**Formato del pack:** spritesheet PNG por tileset, 10 tiles en fila en orden fijo:
```
↖ ↑ ↗ ← # → ↙ ↓ ↘ =
 0  1  2  3 4  5  6  7  8 9
```

Cada tile ocupa `TILE_SCREEN_SIZE x TILE_SCREEN_SIZE` px. Un solo `LoadTexture()` por nivel.

**Ubicación:** `assets/tilesets/<nombre>/tileset.png`

**Responsabilidades:**
- `level_loader.c` lee `tileset=<name>` y lo guarda en `LevelData.tileset`. Ciego a assets.
- `game.c` recibe el string, carga la textura al iniciar la escena, la libera al salir.
- El renderer de plataformas mapea `spriteIndex` a `DrawTextureRec()`.

**Prerequisito:** ninguno, `LevelData.tileset` ya existe.

### Plataformas especiales

**Plataforma rompible**
- Token `X` en el grid.
- `Struct: CrumblingPlatform { rect, state, timer }`
- Estados: `IDLE -> SHAKING -> BROKEN`. Respawn opcional por nivel.
- Colisión desactivada en estado `BROKEN`.

### Sistema de condiciones de victoria
- Definidas en la metadata del nivel, evaluadas cada frame por `game.c`.
- Soporte para condiciones AND y OR.
- Condiciones previstas:
  - `win=kill_all`
  - `win=collect_keys:N`
  - `win=break_all_boxes`
  - `win=reach_exit`
  - `win=kill_boss`

### Limpieza pendiente (game.c)
- Eliminar `static char levelNext[64]` — reliquia, nunca se usa.
- `runner_update` recibe `platformCount` en vez de `totalPlatforms` — bug silencioso con moving platforms.
- `GRID_COLS` / `GRID_ROWS` son statics globales en `level_loader.c` — riesgo si el Scene Manager carga niveles en paralelo.

---

## Completado
- Arquitectura base: scene system, módulos ciegos, zero heap
- `TILE_SIZE` centralizado — todas las dimensiones son ratios
- Culling rectangular: render (viewport) y lógica (3x viewport)
- Player: movimiento, salto, agacharse, sistema de apuntado direccional
- Runner: patrulla, detección, memoria, aggro, drops al morir
- Sistema de armas polimórfico: Semiauto, Shotgun, Full-Auto, Lanzallamas
- Pool de balas, sistema de drops con física
- Cámara con deadzone, lerp, clamp de nivel
- Level loader data-driven desde `.txt` con cabecera de metadata
- Cabecera: `tileset`, `exit:N`, `moving:N`
- Tokens de tile: `1-9` (numpad direccional), `=` (aislado), `B`, `R`, `M`, `E`, `P`
- Transición de niveles via tile `E`
- Plataformas móviles: ping-pong con accel/decel, formato `dx,dy` relativo al spawn, rider logic
- Arrays escalados a 2048 (plataformas, boxes, runners)
- Kill volume para player, runners y drops
- Flag `-DDEBUG` para HUD y overlays
- Player respawnea con loadout inicial al morir
- Drop chance dinámico por kills, tope en 80%
- Scene Manager primitivo: `scene_manager_set / update / render`
- Menú principal: navegación por teclado, Main / Settings / Resolution
