# 2D Data-Driven Platformer
### C + Raylib - Modular, drop-in, zero heap growth

```
┌─────────────────────────────────────────────────────────┐
│  1280×720 px  │  25px/tile  │  C99  │  Raylib           │
└─────────────────────────────────────────────────────────┘
```

> A 2D platformer built on strict separation of concerns.
> Every system is isolated, replaceable, and blind to the others.
> The game loop conducts. Modules execute.

---

## Project Layout

```
.
├── assets/
│   └── levels/
│       ├── level1.txt
│       └── level2.txt
├── entities/
│   ├── box.c / .h
│   ├── bullet.c / .h
│   ├── collision.c / .h
│   ├── drop.c / .h
│   ├── hittable.h
│   ├── player.c / .h
│   └── runner.c / .h
├── scenes/
│   ├── camera.c / .h
│   ├── game.c / .h
│   ├── level_loader.c / .h
│   ├── menu.c / .h
│   ├── moving_platform.c / .h
├── defines.h
├── scene.h
├── scene_manager.c / .h
├── ROADMAP.md
└── main.c
```

---

## Architecture

### Scene Manager

A minimal scene dispatcher. No registry, no stack.

```c
scene_manager_set(menu_scene());   // switch to any scene
scene_manager_update(dt);
scene_manager_render();
```

`main.c` boots into `menu_scene()`. Any scene can transition to any other via `scene_manager_set`.

### Data-Driven Level Loading

Levels are plain-text `.txt` files with an optional metadata header followed by a tile grid.

```
tileset=cave
exit:0=level2.txt
exit:1=level3.txt
moving:0=10,0,3,0.5,0.5,9
---
....P....
88888888.
=........
```

Header lines are `key=value` pairs. The separator `---` marks the start of the grid.
`load_level()` returns a self-contained `LevelData` struct. The scene unpacks it.
The loader has zero knowledge of what the scene does with the data.

**Tile token dictionary:**

```
┌───────┬──────────────────────┬──────────────────────────────────────┐
│ Token │ Entity               │ Description                          │
├───────┼──────────────────────┼──────────────────────────────────────┤
│   .   │ Air                  │ Passable - no allocation             │
│   P   │ Player Spawn         │ Initial origin vector                │
│ 1-9   │ Solid Geometry       │ Directional tile variants (numpad)   │
│   =   │ Isolated Tile        │ Solid tile, no neighbors assumed     │
│   B   │ Destructible Box     │ Stomp-bounce breakable obstacle      │
│   R   │ Runner               │ Patrolling hazard entity             │
│   M   │ Moving Platform      │ Spawn position; path from header     │
│   E   │ Exit Tile            │ Level transition trigger             │
└───────┴──────────────────────┴──────────────────────────────────────┘
```

**Numpad tile layout (maps to spritesheet index):**

```
7 8 9      ↖ ↑ ↗
4 5 6  →   ← # →
1 2 3      ↙ ↓ ↘
=          isolated
```

**Header key reference:**

```
tileset=<name>                            Tileset folder name
exit:N=<filename>                         Nth exit destination
moving:N=dx,dy,speed,accel,decel,sprite   Nth moving platform — offset in tiles from spawn
```

`M` tokens in the grid are matched to `moving:N` entries by parse order.
`dx` and `dy` are signed tile offsets from the `M` position in the grid.
A `M` with no corresponding header entry becomes a static tile with `spriteIndex=9`.

### Module Isolation

Each system owns its logic completely. `game.c` passes pointers and receives results.
Nothing reaches across module boundaries without an explicit interface.

`runner.c` has no knowledge of level geometry - it receives platform arrays as parameters.
`collision.c` has no knowledge of entity types - it operates on `Rectangle` and `Hittable`.
`drop.c` handles probability, physics, and collection internally.

### TILE_SIZE - Single Source of Truth

All world-space dimensions, velocities, forces, and entity sizes derive from `TILE_SIZE`
defined in `defines.h`.

```c
#define TILE_SIZE   25.0f
// Everything else is a ratio
#define MOVE_SPEED  (TILE_SIZE * 7.5f)
#define GRAVITY     (TILE_SIZE * 20.0f)
```

Camera zoom is derived at init time: `cam->zoom = 40.0f / TILE_SIZE`.
Viewport tile count is therefore constant regardless of `TILE_SIZE` value.

### Polymorphic Weapon Dispatch

Weapons bind a `fireFunc` callback at creation time.

```c
currentWeapon->fireFunc(fireOrigin, aimDir, bullets, maxBullets);
```

### Runner AI

```
PATROL → FREEZE (1s) → AGGRO → MEMORY (last known position) → PATROL
```

Detection range doubles on AGGRO. Runners respect gravity, check edges, and jump toward elevated targets.

### Moving Platforms

Ping-pong lerp between spawn position and an offset target, with independent accel/decel ramps.
`tVelocity` is modulated per frame against a trapezoidal velocity profile.
Rider logic: `Player.groundPlatformIndex` tracks which platform tile the player is standing on.
Delta is applied at the start of the next frame before physics integration.

### Culling System

```c
Rectangle camera_get_viewport(const Camera2D *cam);
Rectangle camera_get_logic_bounds(const Camera2D *cam);
```

Entities outside viewport are not rendered.
Entities outside logic bounds (3x viewport) skip update entirely.

### Weapon Drop System

Base 5% drop chance on runner death, +1% per kill, capped at 80%.

| Weapon       | Weight |
|--------------|--------|
| Shotgun      | 75     |
| Full-Auto    | 50     |
| Flamethrower | 25     |

### Kill Volume

Any entity below `levelHeight` is immediately removed or respawned.

---

## Weapon Profiles

| Weapon       | Mechanic                                          | Mode  | Damage     |
|--------------|---------------------------------------------------|-------|------------|
| Semi-Auto    | Single high-speed linear shot per tap             | Press | 34         |
| Shotgun      | 5-pellet burst with angular spread                | Press | 15/pellet  |
| Full-Auto    | Continuous stream with perpendicular recoil       | Hold  | 10         |
| Flamethrower | Dense short-range particle cluster with decay     | Hold  | 5          |

---

## Controls

```
  ┌───┐                      ┌───┐
  │ W │  Jump / Aim Up        │ Q │  Swap weapon (if slot 2 filled)
  └───┘                      └───┘
┌───┬───┐                 ┌───────┐
│ A │ D │  Move            │ Space │  Fire active weapon
└───┴───┘                 └───────┘
  ┌───┐
  │ S │  Crouch (ground) / Aim Down (air)
  └───┘
```

---

## Memory Model

All instance arrays are statically bounded. No heap allocation at runtime.

```
platforms[2048]   boxes[2048]   runners[2048]   bulletPool[100]   drops[32]   movingPlatforms[64]
      ↓                ↓              ↓                ↓               ↓               ↓
[ deterministic bounds - zero heap growth during gameplay ]
```

---

## Build

```bash
# Release
gcc main.c scene_manager.c scenes/game.c scenes/menu.c scenes/level_loader.c scenes/camera.c scenes/moving_platform.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm

# Debug
gcc -DDEBUG main.c scene_manager.c scenes/game.c scenes/menu.c scenes/level_loader.c scenes/camera.c scenes/moving_platform.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm
```

`-DDEBUG` activates:
- Runner state overlay (PATROL / FREEZE / AGGRO / MEMORY + HP)
- Edge raycast and detection range visualization
- Culling HUD: runners rendered vs logic-active vs total
- Level metadata HUD: tileset, exits, moving platform count

> Dependencies: [Raylib](https://www.raylib.com/)

---

## License

See [`LICENSE`](./LICENSE).
