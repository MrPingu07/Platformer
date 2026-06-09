# Platformer en C + Raylib

Prototipo de plataformero 2D escrito en C con raylib. El objetivo del proyecto no es solo que funcione, sino que sea modular, legible y fácil de extender.

---

## Filosofía

El proyecto sigue una arquitectura inspirada en Godot: todo ocurre dentro de **escenas**, y cada escena instancia las **entidades** que necesita. No hay lógica hardcodeada en `main.c`.

- Cada escena tiene su propio loop de `update` y `render`.
- Las entidades son structs con funciones asociadas (`init`, `update`, `render`). Sin objetos, sin magia: solo datos y funciones explícitas.
- `main.c` inicializa la ventana, elige la escena activa y ejecuta su loop. Nada más.

---

## Estructura

```
Platformer/
├── main.c                  — Entry point. Inicializa ventana y ejecuta la escena activa.
├── scene.h                 — Define el tipo Scene (punteros a update y render).
├── entities/
│   ├── player.c / player.h — Struct Player, init, update, render.
│   ├── enemy.c  / enemy.h  — Struct Enemy, init, update, render.
│   └── box.c    / box.h    — Struct Box, init, render.
└── scenes/
    ├── game.c  / game.h    — Escena principal. Instancia entidades y maneja colisiones.
```

---

## Cómo funciona una escena

`Scene` es un struct con dos punteros a funciones:

```c
typedef struct {
    void (*update)(float dt);
    void (*render)(void);
} Scene;
```

Cada escena expone una función que la inicializa y devuelve una `Scene`:

```c
Scene game_scene(void);
```

`main.c` la usa así:

```c
Scene current = game_scene();

while (!WindowShouldClose()) {
    current.update(GetFrameTime());
    current.render();
}
```

Para agregar una escena nueva: crear `scenes/nueva.c` y `scenes/nueva.h`, implementar `update` y `render`, y asignarla a `current` en `main.c`.

---

## Cómo funciona una entidad

Cada entidad es un struct con datos propios y funciones externas que los manipulan:

```c
// Declaración (header)
typedef struct {
    float x, y;
    float vy;
    bool on_ground;
} Player;

Player player_init(float x, float y);
void   player_update(Player *p, float dt);
void   player_render(Player *p);
```

Las funciones reciben un puntero (`*p`) para poder modificar el original. Sin puntero, C pasa una copia y los cambios se pierden.

Para agregar una entidad nueva: crear su par `.c/.h` con `init`, `update` y `render`, e incluirla en la escena que la necesite.

---

## Física y colisiones

La física es intencional y simple:

- Gravedad: cada frame se suma a `vy`, que acumula velocidad hacia abajo.
- El jugador no sabe si está en el suelo. `on_ground` se resetea a `false` al inicio de cada frame y las colisiones lo corrigen a `true` si corresponde.
- El input de salto vive en `game_update`, después de las colisiones, para leer `on_ground` con el valor correcto.

Tipos de colisión implementados:
- Jugador con suelo (línea fija).
- Jugador con plataformas (solo desde arriba).
- Jugador con cajas (desde arriba, rompe la caja y rebota).
- Jugador con enemigos (pisarlos los mata, contacto lateral resetea al jugador).

---

## Compilación

```bash
gcc main.c scenes/game.c entities/player.c entities/enemy.c entities/box.c entities/bullet.c -o platformer -lraylib -lm
```

Ejecutar desde la carpeta `Platformer/`.

---

## Dependencias

- [raylib](https://www.raylib.com/) — `sudo pacman -S raylib`
- [clang / clangd](https://clangd.llvm.org/) — LSP para el editor — `sudo pacman -S clang`

---

## Estado actual

- Jugador con movimiento horizontal, salto y gravedad.
- Plataformas con colisión.
- Cajas destruibles.
- Enemigo con patrulla horizontal y colisión tipo Mario.
- Cambio de escena en runtime: pendiente.# Platformer
 
