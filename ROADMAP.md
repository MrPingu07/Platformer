# Roadmap
 
## En progreso
_nada actualmente_
 
---
 
## Pendiente
- **Roto: win=kill_all** La condicion win=kill_all se cumple y el tile se colorea de blanco, pero no al tocarla el player no funciona..
 
### Enemigos
- **Gunner** — Variante del Runner. Mantiene distancia del jugador, dispara ráfagas tipo semiauto. Reutiliza `bullet.c` y `Hittable`.
- **Mosquito** — Patrullero volador. Ignora geometría del mundo. Al detectar al jugador carga en patrón errático. Sin gravedad.

**Tunneling** Mas notablemente el player y los runners lo sufren. Probablemente ocurre con los Drops.

**Tamaño de drops** Los drops no escalen bien con el grid tilesize. Con un tilesize 100 son gigantes.

**Colisiones laterales** Al menos respecto al player y los runners (Probablemente los drops, y todo, realmente), la colision de los tiles solidos es traspasable totalmente desde los costados. Funcionalmente los tiles con colosion solo las tienen desde arriba.

### Escenas
- **Scene Manager** — Implementado en forma primitiva. Sin registro por ID ni pila. Expandir cuando haya más escenas que lo justifiquen.
- **Menú principal** — Implementado: Main / Settings / Resolution. Pendiente: conectar resoluciones reales con `SetWindowSize`.
### Metadata de nivel
- **Victory conditions** — `win=kill_all` implementado. Pendiente:
  - `win=collect_keys:N`
  - `win=break_all_boxes`
  - `win=reach_exit`
  - `win=kill_boss`
  - Soporte AND/OR entre condiciones
### Sistema de Tilesets
El designer coloca los tiles manualmente en el `.txt`. El engine no infiere vecinos.
 
**Formato del pack:** spritesheet del mundo PNG por tileset, hasta 10 tiles por fila en orden fijo: vvv
```Fila 1
↖ ↑ ↗ ← # → ↙ ↓ ↘ =
0 1 2 3 4 5 6 7 8 9
```
```Fila 2
K - E
Key - Exit
```
**Sujeto a cambios** ^^^
 
**Ubicación:** `assets/tilesets/<nombre>/tileset.png`
 
**Responsabilidades:**
- `level_loader.c` lee `tileset=<name>` y lo guarda en `LevelData.tileset`. Ciego a assets.
- `game.c` recibe el string, carga la textura al iniciar la escena, la libera al salir.
- El renderer de plataformas mapea `spriteIndex` a `DrawTextureRec()`.
- Exit tile: WHITE si condición cumplida, RED si bloqueada. Reemplazar con sprite cuando llegue el sistema.

### Plataformas especiales
 
**Plataforma rompible**
- Token `X` en el grid.
- `Struct: CrumblingPlatform { rect, state, timer }`
- Estados: `IDLE -> SHAKING -> BROKEN`. Respawn opcional por nivel.
- Colisión desactivada en estado `BROKEN`.

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
- Cabecera: `tileset`, `exit:N`, `moving:N`, `win`
- Tokens de tile: `1-9`, `=`, `B`, `R`, `M`, `E`, `P`
- Transición de niveles via tile `E`
- Exit tile bloqueado visualmente si condición no cumplida
- Plataformas móviles: ping-pong con accel/decel, formato `dx,dy`, rider logic
- Arrays escalados a 2048 (plataformas, boxes, runners)
- Kill volume para player, runners y drops
- Flag `-DDEBUG` para HUD y overlays
- Player respawnea con loadout inicial al morir
- Drop chance dinámico por kills, tope en 80%
- Scene Manager: `scene_manager_set / update / render`
- Menú principal: navegación por teclado, Main / Settings / Resolution
- `SetExitKey(KEY_NULL)` — ESC libre para lógica del juego
- Refactorización game.c: `GameState`, `game_init`, `game_update`, `game_render` como módulos separados
- `GRID_COLS/GRID_ROWS` eliminados como globals, reemplazados por `GridSize` local
- Runner colisión corregida: recibe `totalPlatforms` en vez de `platformCount`
