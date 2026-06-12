# Roadmap

## En progreso
_nada actualmente_

---

## Pendiente

### Estructura
- **Reduccion del mundo** Reducir el tamaño estandar de las entidades y las plataformas entre otros de 40px a 10px para tener de base un numero mas limpio, multiplicable y divisible. La cámara tambien se ajustara correspondientemente.

### Enemigos
- **Gunner** - Variante del Runner. Mantiene distancia del jugador, dispara ráfagas de balas tipo semiauto. Reutiliza infraestructura de `bullet.c` y `Hittable`.
- **Mosquito** - Patrullero volador. Ignora geometría del mundo. Al detectar al jugador carga en patrón errático. Sin gravedad.
- **Cambio de Runner en TXT** Runner actualmente se parsea con una E (de Enemy, legacy) en los txt. Pendiente de cambiar a R.

### Escenas
- **Menú principal** - Requiere sistema de transición entre escenas en `main.c`. Prerequisito para cualquier otra escena.

---
## Completado
- Arquitectura base: scene system, módulos ciegos, zero heap
- Player: movimiento, salto, agacharse, sistema de apuntado direccional
- Runner: patrulla, detección, memoria, aggro, chance de drops al morir
- Sistema de armas polimórfico: Semiauto, Shotgun, Full-Auto, Lanzallamas
- Pool de balas, sistema de drops con física
- Cámara con deadzone y lerp
- Level loader data-driven desde `.txt`
- Culling rectangular: render (viewport exacto) y lógica (3x viewport)
- Arrays escalados a 2048
- Kill volume para player, runners y drops
- Flag `DEBUG` en compilación para HUD y overlays de debug
