# Roadmap

## En progreso — Bug activo
**Bloqueo total de input horizontal en el aire al tocar pared lateral.**

`resolve_horizontal_collisions` corrige `p->x` cada frame mientras haya overlap con
la zona delgada de pared (`t = 3.0f`), incluso cuando `player_integrate_x` ya movió
al player en dirección de salir. La aceleración de aire (`AIR_ACCELERATION`) es
insuficiente para escapar los 3px de la zona en un solo frame, generando un loop de
re-detección → re-corrección que ancla al player hasta tocar el suelo.

`vx` sí cambia correctamente frame a frame (fricción, input) — el problema es que la
posición se fuerza de vuelta al borde sin importar hacia dónde apunta el movimiento.

Próximo intento: guardar `prevX` (igual que `prevY`) y solo corregir posición si el
player se movió genuinamente hacia adentro de la pared respecto al frame anterior,
no solo por overlap estático.

---

## Pendiente

### Bugs
- Bloqueo de input horizontal en el aire (ver arriba) — prioridad alta.
- Tunneling a velocidades altas — player, runners, posiblemente drops.
- Runners y drops no tienen colisión horizontal por eje separado, solo vertical.
- Colisión horizontal no aplica rider logic a plataformas móviles (no detecta pared
  si el player está parado sobre una plataforma en movimiento).

### Enemigos
- Gunner — variante de Runner, mantiene distancia, ráfagas semiauto.
- Mosquito — volador, sin gravedad, patrón errático al detectar jugador.

### Escenas
- Scene Manager: expandir con registro por ID si se agregan más escenas.
- Menú de pausa — pendiente. Llamar `camera_init` si se cambia resolución mid-game.

### Metadata de nivel
`win=kill_all` funcional. Pendientes: `collect_keys:N`, `break_all_boxes`,
`reach_exit`, `kill_boss`, soporte AND/OR entre condiciones.

### Tilesets
Formato definido (spritesheet PNG, 10 tiles/fila), loader ciego a assets — pendiente
implementación real de carga/render por tileset. Exit tile usa color placeholder
(WHITE/BLACK) hasta que llegue el sistema de sprites.

### Plataformas especiales
Plataforma rompible (token `X`, estados IDLE→SHAKING→BROKEN) — no implementada.

---

## Completado
- Arquitectura base: scene system, módulos ciegos, zero heap
- `TILE_SIZE` centralizado, culling render/lógica
- Player: movimiento, salto, agachado, apuntado direccional, armas polimórficas
- Runner AI completo con drops al morir
- Level loader data-driven, header de metadata, todos los tokens de tile
- Sistema de resolución (aspect ratios, window modes)
- Level select paginado
- Refactor game.c en submódulos (init/update/render/state)
- Physics pipeline separado por eje (X e Y integrados independientemente)
- One-way platforms verticales (estáticas y móviles, funciones separadas)
- Colisión horizontal con detección de vecino (solo caras expuestas al aire)
- Fix: win condition invertida en game_update
- Fix: tamaño de partículas del flamethrower
- Fix: doble integración de gravedad/Y (bug de duplicación de llamada)
