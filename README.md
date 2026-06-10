# 2D Data-Driven Platformer
### C + Raylib — Pure systems, minimal bloat

```
┌─────────────────────────────────────────────────────────┐
│  640×480 px  │  16×12 tiles  │  40×40 px/tile  │  C99  │
└─────────────────────────────────────────────────────────┘
```

> A modular 2D platformer built on strict separation of concerns.
> Level geometry, hazard states, combat profiles and player mechanics
> live in isolated modules the game loop just conducts.

---

## Project Layout

```
.
├── assets/
│   └── levels/
│       └── level1.txt          # Plain-text spatial matrix
├── entities/
│   ├── box.c / .h              # Breakable obstacles + bounce physics
│   ├── bullet.c / .h           # Projectile pool + weapon profiles
│   ├── runner.c / .h           # Patrol + gravity + stomp/damage states
│   └── player.c / .h           # State machine, physics, aim system, inventory
├── scenes/
│   └── game.c / .h             # Scene coordinator + tilemap parser
├── scene.h                     # Scene interface (Update/Render fn ptrs)
├── main.c                      # Entry point — window + lifecycle
└── platformer                  # Compiled binary
```

---

## Engine Philosophy

### Data-Driven Execution
Level maps and world objects are never hardcoded. The engine parses
plain-text `.txt` matrices at runtime. Swap a file, get a new level.
Every entity — platforms, boxes, runners, player spawn — is derived
from tile tokens at load time.

### Complete System Encapsulation
`game.c` is a pure executive controller. `Runner` and `Box` entities own
their transformation loops, state flags and collision resolution entirely.
Coordination happens through pointer injection, not globals.

World collision resolution is handled by `game.c` and passed to entities
as needed, so entity modules have zero knowledge of level geometry.
A runner reacts to whatever tileset is loaded at that moment.

### Polymorphic Weapon Dispatch
No `if/else` chains in the main loop. Each weapon binds a `fireFunc`
callback that executes its own ballistic profile independently.

```c
// Weapon bound at load time — dispatched polymorphically at runtime
weapon.fireFunc(fireOrigin, aimDir, bullets, maxBullets);
```

### Directional Aim System
Fire direction is resolved each frame from a normalized `Vector2 aimDir`
derived from input state and player context. Weapons receive a direction
vector, not a raw integer, making diagonal and vertical fire a natural
extension of the same dispatch path.

---

## Tilemap Token Dictionary

```
┌───────┬─────────────────────┬──────────────────────────────────────┐
│ Token │ Entity              │ Description                          │
├───────┼─────────────────────┼──────────────────────────────────────┤
│   .   │ Air Space           │ Passable buffer — no physics alloc   │
│   P   │ Player Spawn        │ Initial origin vector for the player │
│  # =  │ Solid Geometry      │ Static platforms and boundaries      │
│   B   │ Destructible Box    │ Stomp-bounce breakable obstacle      │
│   E   │ Runner              │ Patrolling hazard — stomp or damage  │
└───────┴─────────────────────┴──────────────────────────────────────┘
```

Example matrix fragment:

```
................
###.....E....###
........B.......
P...............
================
```

---

## Weapon Profiles

| Weapon        | Mechanic                                           | Mode  | Damage |
|---------------|----------------------------------------------------|-------|--------|
| Semi-Auto     | Single high-speed linear shot per tap              | Press | 34     |
| Shotgun       | 5-pellet burst with symmetric angular spread       | Press | 15/pellet |
| Full-Auto     | Continuous stream with perpendicular recoil spread | Hold  | 10     |
| Flamethrower  | Dense short-range particle cluster with decay      | Hold  | 5      |

Projectile lifecycle is managed by a centralized bullet memory pool
(`MAX_BULLETS 100`). Allocation and deallocation never touch the heap at runtime.

Each bullet carries a `damage` value set at spawn time by its weapon profile.
Hit detection and health resolution happen in `game.c`, keeping entity modules
free of cross-entity logic.

---

## Runner

Runners patrol horizontally, respect gravity, and react to the loaded tileset.
They carry 100 HP and die either from bullet damage or a player stomp.

| Condition          | Result                                  |
|--------------------|-----------------------------------------|
| Player stomps      | Runner dies, player gets upward bounce  |
| Player touches     | Player reset to spawn                   |
| Bullet hit         | Runner loses `bullet.damage` HP         |
| HP reaches 0       | Runner marked dead, removed from render |

---

## Controls

```
  ┌───┐                      ┌───┐
  │ W │  Jump / Aim Up        │ Q │  Swap weapon slot
  └───┘                      └───┘
┌───┬───┐                 ┌───────┐
│ A │ D │  Move            │ Space │  Fire active weapon
└───┴───┘                 └───────┘
  ┌───┐
  │ S │  Crouch (ground) / Aim Down (air)
  └───┘
```

### Aim Direction Matrix

```
         W
    W+A  ↑  W+D
      ↖  │  ↗
  A ←────●────→ D     (ground or air)
      ↙  │  ↘
    S+A  ↓  S+D       (air only)
         S
```

Holding `W` on the ground with no horizontal input locks the X axis.
Crouching reduces movement speed by half and shifts the muzzle origin downward.
Diagonal and downward fire is only available in the air.

---

## Build

```bash
gcc main.c scenes/game.c entities/player.c entities/runner.c \
    entities/box.c entities/bullet.c \
    -o platformer -lraylib -lm
```

> Dependencies: [Raylib](https://www.raylib.com/) — `pacman -S raylib`

---

## Memory Model

Instance counts are tracked via live index registers at runtime:

```
platformCount   boxCount   runnerCount   bulletPool[MAX_BULLETS]
     ↓               ↓           ↓               ↓
[ deterministic bounds — no heap growth during gameplay ]
```

All live instance arrays are statically bounded to maintain predictable
memory footprints and eliminate leak vectors in long sessions.

---

## License

See [`LICENSE`](./LICENSE).
