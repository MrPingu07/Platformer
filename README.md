# 2D Data-Driven Platformer
### C + Raylib - Modular, drop-in, zero heap growth

```
┌─────────────────────────────┐
│  C99  │  Raylib  │  256×240 │
└─────────────────────────────┘
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
│   ├── game_init.c / .h
│   ├── game_render.c / .h
│   ├── game_state.h
│   ├── game_update.c / .h
│   ├── level_loader.c / .h
│   ├── level_select.c / .h
│   ├── menu.c / .h
│   ├── moving_platform.c / .h
│   ├── resolution.c / .h
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
scene_manager_set(menu_scene());
scene_manager_update(dt);
scene_manager_render();
```

`main.c` boots into `menu_scene()`. Any scene can transition to any other via `scene_manager_set`.

### Data-Driven Level Loading

Levels are plain-text `.txt` files with an optional metadata header followed by a tile grid.

```
tileset=cave
win=kill_all
exit:0=level2.txt
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
win=<condition>                           Victory condition
exit:N=<filename>                         Nth exit destination
moving:N=dx,dy,speed,accel,decel,sprite   Nth moving platform
```

`M` tokens in the grid are matched to `moving:N` entries by parse order.
`dx` and `dy` are signed tile offsets from the `M` position in the grid.
A `M` with no corresponding header entry becomes a static tile with `spriteIndex=9`.

### Resolution System

The game renders at a fixed internal resolution of **256×240** (NES-style).
Window size is a multiple of that base that fits the monitor.

**Aspect ratios** change how much world is visible horizontally. The camera
and level boundaries are unaffected; wider ratios simply reveal more background.

```
4:3  →  320×240
3:2  →  360×240
16:9 →  426×240
21:9 →  560×240
32:9 →  853×240
```

**Window modes:** Windowed (integer scale), Borderless, Fullscreen.
Configured via Settings → Resolution in the main menu. Applied immediately on ENTER.

### Level Select

`level_select_scene()` scans `assets/levels/` for `.txt` files at entry,
sorts them alphabetically, and displays them in a paged list (`LEVELS_PER_PAGE = 8`).
Memory is cleared both on selection (before the level loads) and on ESC (before
returning to the menu). No heap involved.

### Module Isolation

Each system owns its logic completely. `game.c` passes pointers and receives results.
Nothing reaches across module boundaries without an explicit interface.

`runner.c` has no knowledge of level geometry. It receives platform arrays as parameters.
`collision.c` has no knowledge of entity types. It operates on `Rectangle` and `Hittable`.
`drop.c` handles probability, physics, and collection internally.

### game.c Submodules

`game.c` was refactored into four focused files:

```
game_state.h    Shared GameState struct
game_init.c     World setup from LevelData
game_update.c   Per-frame logic, physics, transitions
game_render.c   Draw calls, HUD, debug overlays
```

### TILE_SIZE - Single Source of Truth

All world-space dimensions, velocities, forces, and entity sizes derive from `TILE_SIZE`
defined in `defines.h`.

```c
#define TILE_SIZE   25.0f
#define MOVE_SPEED  (TILE_SIZE * 7.5f)
#define GRAVITY     (TILE_SIZE * 20.0f)
```

Camera zoom is derived at init time: `cam->zoom = 40.0f / TILE_SIZE`.
Viewport tile count is therefore constant regardless of `TILE_SIZE` value.

### Physics Pipeline (Player)

Player physics are resolved in explicit axis-separated steps each frame:

```
player_handle_input()                input → vx, vy (jump), isCrouching
player_integrate_x()                 p->x += vx * dt
player_apply_gravity()               vy += GRAVITY * dt
prevY = p->y
player_integrate_y()                 p->y += vy * dt
resolve_environment_collisions()     one-way vertical (static platforms, uses prevY)
resolve_moving_platform_collisions() vertical (moving platforms, margin-based)
resolve_horizontal_collisions()      lateral walls (neighbor-aware, exposed faces only)
```

Separating X and Y integration eliminates diagonal tunneling artifacts and allows
per-axis collision rules without ambiguity.

### Collision Model

Tiles are **one-way from above**: the player can pass through from below and land
from above, using `prevY` to detect entry direction.

Horizontal collision only blocks on **exposed faces** - a tile checks its neighbors
in the same row at load-resolve time; if an adjacent tile shares that side, the wall
is skipped. Contiguous rows of tiles behave as a single seamless surface rather than
a grid of individual walls.

Static and moving platforms use separate resolution functions because moving
platforms require a margin-based approach immune to frame-rate-dependent position deltas.

**Known issue:** horizontal wall resolution can pin the player's horizontal input
while airborne and in contact with a lateral wall, since air acceleration may be too
low to escape the contact zone before the next frame re-triggers correction. Tracked
in ROADMAP.

### Polymorphic Weapon Dispatch

Weapons bind a `fireFunc` callback at creation time.

```c
currentWeapon->fireFunc(fireOrigin, aimDir, bullets, maxBullets);
```

### Runner AI

```
PATROL → FREEZE (1s) → AGGRO → MEMORY (last known position) → PATROL
```

Detection range doubles on AGGRO. Runners respect gravity, check edges,
and jump toward elevated targets.

### Moving Platforms

Ping-pong lerp between spawn position and an offset target, with independent
accel/decel ramps. `tVelocity` is modulated per frame against a trapezoidal
velocity profile. Rider logic: `Player.groundPlatformIndex` tracks which platform
the player is standing on. Delta is applied at the start of the next frame before
physics integration.

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

Level name list (level select): `char[256][64]`, static, cleared on exit.

---

## Build

```bash
# Release
gcc main.c scene_manager.c \
    scenes/game.c scenes/game_init.c scenes/game_update.c scenes/game_render.c \
    scenes/menu.c scenes/level_loader.c scenes/camera.c scenes/moving_platform.c \
    scenes/level_select.c scenes/resolution.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm

# Debug
gcc -DDEBUG main.c scene_manager.c \
    scenes/game.c scenes/game_init.c scenes/game_update.c scenes/game_render.c \
    scenes/menu.c scenes/level_loader.c scenes/camera.c scenes/moving_platform.c \
    scenes/level_select.c scenes/resolution.c \
    entities/player.c entities/runner.c entities/box.c \
    entities/bullet.c entities/collision.c entities/drop.c \
    -o platformer -lraylib -lm
```

`-DDEBUG` activates:
- Runner state overlay (PATROL / FREEZE / AGGRO / MEMORY + HP)
- Edge raycast and detection range visualization
- Culling HUD: runners rendered vs logic-active vs total
- Level metadata HUD: tileset, win condition

> Dependencies: [Raylib](https://www.raylib.com/)

---

## License

See [`LICENSE`](./LICENSE).
