# 2D Data-Driven Platformer
### C + Raylib - Modular, drop-in, zero heap growth

```
┌─────────────────────────────────────────────────────────┐
│  640×480 px  │  25px/tile  │  C99  │  Raylib 6.0        │
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
│       └── level1.txt           # Plain-text spatial matrix
├── entities/
│   ├── box.c / .h               # Breakable obstacles + bounce physics
│   ├── bullet.c / .h            # Projectile pool + polymorphic weapon profiles
│   ├── collision.c / .h         # Environment, rect, and hittable resolution
│   ├── drop.c / .h              # Weapon drop physics, probability, collection
│   ├── hittable.h               # Generic damageable entity interface
│   ├── player.c / .h            # Physics, movement, aim system, inventory
│   └── runner.c / .h            # Patrol AI, detection, memory, aggro states
├── scenes/
│   ├── camera.c / .h            # Deadzone tracking, lerp, level clamp, culling rects
│   ├── level_loader.c / .h      # Tilemap parser - returns LevelData struct
│   └── game.c / .h              # Scene coordinator - init, update, render
├── defines.h                    # Global constants - TILE_SIZE, TILE_SCREEN_SIZE, shared defines
├── scene.h                      # Scene interface (Update/Render fn ptrs)
├── ROADMAP.md                   # Feature backlog and priorities
└── main.c                       # Entry point - window + lifecycle
```

---

## Architecture

### Data-Driven Level Loading

Levels are plain-text `.txt` tile matrices parsed at runtime by `level_loader.c`.
No entity is hardcoded. Swap the file, get a new world.

`load_level()` returns a self-contained `LevelData` struct. The scene unpacks it.
The loader has zero knowledge of what the scene does with the data.

```c
LevelData data = load_level("assets/levels/level1.txt");
// platforms, runners, boxes, player spawn - all derived from the tile matrix
```

Tile token dictionary:

```
┌───────┬─────────────────────┬──────────────────────────────────────┐
│ Token │ Entity              │ Description                          │
├───────┼─────────────────────┼──────────────────────────────────────┤
│   .   │ Air                 │ Passable buffer - no allocation      │
│   P   │ Player Spawn        │ Initial origin vector                │
│  # =  │ Solid Geometry      │ Static platforms and boundaries      │
│   B   │ Destructible Box    │ Stomp-bounce breakable obstacle      │
│   R   │ Runner              │ Patrolling hazard entity             │
└───────┴─────────────────────┴──────────────────────────────────────┘
```

### Module Isolation

Each system owns its logic completely. `game.c` passes pointers and receives results.
Nothing reaches across module boundaries without an explicit interface.

`runner.c` has no knowledge of level geometry - it receives platform arrays as parameters.
`collision.c` has no knowledge of entity types - it operates on `Rectangle` and `Hittable`.
`drop.c` handles probability, physics, and collection internally - `game.c` calls one function.

### TILE_SIZE - Single Source of Truth

All world-space dimensions, velocities, forces, and entity sizes derive from `TILE_SIZE`
defined in `defines.h`. Changing one value rescales the entire simulation.

```c
// defines.h
#define TILE_SIZE        25.0f
#define TILE_SCREEN_SIZE 40.0f

// Everything else is a ratio
#define MOVE_SPEED   (TILE_SIZE * 7.5f)
#define GRAVITY      (TILE_SIZE * 20.0f)
#define RUNNER_SPEED (TILE_SIZE * 2.5f)
```

Camera zoom is derived from `TILE_SCREEN_SIZE / TILE_SIZE`, keeping the visual footprint
of the player constant regardless of `TILE_SIZE`:

```c
cam->zoom = TILE_SCREEN_SIZE / TILE_SIZE;
```

`TILE_SIZE` controls simulation granularity. `TILE_SCREEN_SIZE` controls visual scale.
Both are independent.

### Polymorphic Weapon Dispatch

Weapons bind a `fireFunc` callback at creation time. The fire path is identical for all four profiles - no branching in the game loop.

```c
// Bound at load time, dispatched at runtime
currentWeapon->fireFunc(fireOrigin, aimDir, bullets, maxBullets);
```

### Runner AI

Runners operate across four states driven by detection range and memory timers.

```
PATROL → FREEZE (1s) → AGGRO → MEMORY (last known position) → PATROL
```

Detection range doubles on entering AGGRO. Memory persists for `RUNNER_MEMORY_TIME` seconds
after losing sight. Runners respect gravity, check edges before patrolling off platforms,
and jump toward targets on higher ground.

### Culling System

Two rectangular regions are computed every frame from the active `Camera2D` in `camera.c`.
Both regions operate in world-space and are available to any scene.

```c
Rectangle camera_get_viewport(const Camera2D *cam);     // Exact screen bounds in world-space
Rectangle camera_get_logic_bounds(const Camera2D *cam); // 3x viewport, centered on camera
```

```
┌─────────────────────────────────────────────┐
│             logic bounds (3x)               │
│   ┌─────────────────────────────────────┐   │
│   │                                     │   │
│   │         viewport (1x)               │   │
│   │                                     │   │
│   └─────────────────────────────────────┘   │
│                                             │
└─────────────────────────────────────────────┘
```

Entities outside the viewport are not rendered.
Entities outside the logic bounds skip their update entirely - AI, physics, collision.
Entities inside the logic bounds but outside the viewport update normally, giving the
impression that the world keeps running just beyond the player's view.

### Weapon Drop System

Runners have a base 5% drop chance on death, increasing by 1% per kill and capping at 80%.
Drop type is weighted:

| Weapon       | Weight |
|--------------|--------|
| Shotgun      | 75     |
| Full-Auto    | 50     |
| Flamethrower | 25     |

Drops arc from the runner's position, land on platforms, and wait for collection.
The player starts with Semi-Auto only. The second inventory slot is locked until a
drop is collected. On death, the player respawns with the initial loadout.

### Kill Volume

Any entity - player, runner, or drop - that falls below `levelHeight` is immediately
removed or respawned. No coordinate accumulation, no silent state corruption.

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
platforms[2048]   boxes[2048]   runners[2048]   bulletPool[100]   drops[32]
      ↓                ↓              ↓                ↓               ↓
[ deterministic bounds - zero heap growth during gameplay ]
```

---

## Build

```bash
# Release
gcc main.c scenes/game.c scenes/level_loader.c scenes/camera.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm

# Debug: Add -DDEBUG right after gcc - enables HUD overlays and AI state rendering
```

`-DDEBUG` activates:
- Runner state overlay (PATROL / FREEZE / AGGRO / MEMORY + HP)
- Edge raycast and detection range visualization
- Culling HUD: runners rendered vs logic-active vs total

> Dependencies: [Raylib](https://www.raylib.com/)

---

## License

See [`LICENSE`](./LICENSE).
