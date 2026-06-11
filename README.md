# 2D Data-Driven Platformer
### C + Raylib — Modular, drop-in, zero heap growth

```
┌─────────────────────────────────────────────────────────┐
│  640×480 px  │  40×40 px/tile  │  C99  │  Raylib 6.0   │
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
│   ├── camera.c / .h            # Deadzone tracking + lerp + level clamp
│   ├── level_loader.c / .h      # Tilemap parser — returns LevelData struct
│   └── game.c / .h              # Scene coordinator — init, update, render
├── scene.h                      # Scene interface (Update/Render fn ptrs)
└── main.c                       # Entry point — window + lifecycle
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
// platforms, runners, boxes, player spawn — all derived from the tile matrix
```

Tile token dictionary:

```
┌───────┬─────────────────────┬──────────────────────────────────────┐
│ Token │ Entity              │ Description                          │
├───────┼─────────────────────┼──────────────────────────────────────┤
│   .   │ Air                 │ Passable buffer — no allocation      │
│   P   │ Player Spawn        │ Initial origin vector                │
│  # =  │ Solid Geometry      │ Static platforms and boundaries      │
│   B   │ Destructible Box    │ Stomp-bounce breakable obstacle      │
│   E   │ Runner              │ Patrolling hazard entity             │
└───────┴─────────────────────┴──────────────────────────────────────┘
```

### Module Isolation

Each system owns its logic completely. `game.c` passes pointers and receives results.
Nothing reaches across module boundaries without an explicit interface.

`runner.c` has no knowledge of level geometry — it receives platform arrays as parameters.
`collision.c` has no knowledge of entity types — it operates on `Rectangle` and `Hittable`.
`drop.c` handles probability, physics, and collection internally — `game.c` calls one function.

```c
// game.c on runner death — full drop logic lives in drop.c
drops_try_spawn(gameDrops, MAX_DROPS, x, y);
```

### Polymorphic Weapon Dispatch

Weapons bind a `fireFunc` callback at creation time. The fire path is identical for all four profiles — no branching in the game loop.

```c
// Bound at load time, dispatched at runtime
currentWeapon->fireFunc(fireOrigin, aimDir, bullets, maxBullets);
```

### Directional Aim System

Fire direction resolves each frame from a normalized `Vector2 aimDir` derived from
input state and player context. Weapons receive a direction vector — diagonal and
vertical fire are natural extensions of the same dispatch path.

```
         W
    W+A  ↑  W+D
      ↖  │  ↗
  A ←────●────→ D
      ↙  │  ↘
    S+A  ↓  S+D
         S         (air only)
```

### Runner AI

Runners operate across four states driven by detection range and memory timers.

```
PATROL → FREEZE (1s) → AGGRO → MEMORY (last known position) → PATROL
```

Detection range doubles on entering AGGRO. Memory persists for 10 seconds after
losing sight. Runners respect gravity, check edges before patrolling off platforms,
and jump toward targets on higher ground.

### Weapon Drop System

Runners have a 5% drop chance on death. Drop type is weighted:

| Weapon       | Weight |
|--------------|--------|
| Shotgun      | 75     |
| Full-Auto    | 50     |
| Flamethrower | 25     |

Drops arc from the runner's position, land on platforms, and wait for collection.
The player starts with Semi-Auto only. The second inventory slot is locked until a
drop is collected.

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
platforms[255]   boxes[255]   runners[255]   bulletPool[100]   drops[32]
      ↓               ↓             ↓               ↓               ↓
[ deterministic bounds — zero heap growth during gameplay ]
```

---

## Build

```bash
gcc main.c scenes/game.c scenes/level_loader.c scenes/camera.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm
```

> Dependencies: [Raylib](https://www.raylib.com/)

---

## License

See [`LICENSE`](./LICENSE).
