# Roadmap

## En progreso
_nada actualmente_

---

## Pendiente
- Fix a bug: Si estas parado en una plataforma en movimiento y te toca la cara lateral de otra plataforma en movimiento, no te empuja. Debería. Por ahora solo resetea la aceleracion en eje X del player a 0.

### Bugs
- Snap cosmético menor al pegarse a una pared lateral: si el player se acerca lo
  suficiente y mantiene input hacia la pared, la corrección de posición final se
  aplica de una vez en vez de interpolada, generando un pequeño "tirón" perceptible
  solo si se busca a propósito. No afecta gameplay. Aceptado, no se va a arreglar
  por ahora.
- Enemigos y drops no tienen colisión horizontal por eje separado, solo vertical.

### Enemigos/Hazards
- Gunner - variante de Runner, mantiene distancia, ráfagas semiauto.
- Mosquito - volador, sin gravedad, patrón errático de patrulla y al detectar jugador.
- Grunt - Pendiente.
- 3 Hazards ambientales. Tiles direccionales, (<, >. ^, V. se me ocurren espinas), Sprite movil (ej. Sierra circular), Otro sprite movil (Cualquier cosa).

### Escenas
- Scene Manager: expandir con registro por ID si se agregan más escenas.
- Inventario y progreso general de juego.
- Menú de pausa - pendiente. Llamar `camera_init` si se cambia resolución mid-game.

### Metadata de nivel
`win=kill_all` funcional. Pendientes: `collect_keys:N`, `break_all_boxes`,
`reach_exit`, `kill_boss`, soporte AND/OR entre condiciones.

### Tilesets
Formato definido (spritesheet PNG), loader ciego a assets - pendiente
implementación real de carga/render por tileset. Se usan colores placeholder hasta que llegue el sistema de sprites.

### Plataformas especiales
-Plataforma rompible (token `X`, estados IDLE→SHAKING→BROKEN, limpiar de la memoria) - no implementada.
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
- Colisión horizontal robusta y funcional

