# Roadmap

## En progreso
_nada actualmente_

---

## Pendiente

### Bugs
- Snap cosmético menor al pegarse a una pared lateral: si el player se acerca lo
  suficiente y mantiene input hacia la pared, la corrección de posición final se
  aplica de una vez en vez de interpolada, generando un pequeño "tirón" perceptible
  solo si se busca a propósito. No afecta gameplay. Aceptado, no se va a arreglar
  por ahora.
- Enemigos y drops no tienen colisión horizontal por eje separado, solo vertical.
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
- One-way platforms verticales (estáticas y móviles, funciones separadas, usan prevY)
- **Colisión horizontal robusta y funcional**
  - Detección de vecino por fila: solo caras expuestas al aire bloquean (tiles
    contiguos forman una superficie continua, no una grilla de paredes internas)
  - Corrección solo aplica cuando el movimiento (`vx`) apunta genuinamente hacia
    la pared, no al alejarse — evita el bloqueo de input residual
  - `vx` se resetea a 0 incondicionalmente al chocar (tanto en aire como en suelo),
    eliminando el "arrastre" de velocidad vieja al invertir dirección
  - Guard anti-embebido: compara el rect del player en el frame anterior
    (`prevFrameX`/`prevFrameY`) contra el tile — si ya había overlap, se sigue
    tratando como fantasma. Resuelve el caso de saltar dentro de un tile de borde
    desde abajo sin generar teleports al presionar la dirección de entrada
  - Funciona de forma consistente en suelo y aire, subiendo o cayendo

