# 2D Data-Driven Platformer
### C + Raylib - Pure systems attempting minimal bloat

```
┌─────────────────────────────────────────────────────────┐
│  640×480 px  │  16×12 tiles  │  40×40 px/tile  │  C99  │
└─────────────────────────────────────────────────────────┘
```

> A modular 2D platformer built on **strict separation of concerns**.
> Level geometry, hazard states, combat profiles and player mechanics
> live in isolated modules the game loop just conducts.

---

## 📂 Project Layout

```
.
├── assets/
│   └── levels/
│       └── level1.txt          # Plain-text spatial matrix
├── entities/
│   ├── box.c / .h              # Breakable obstacles + bounce physics
│   ├── bullet.c / .h           # Projectile pool + weapon profiles
│   ├── enemy.c / .h            # Patrol logic + stomp/damage states
│   └── player.c / .h           # State machine, physics, inventory
├── scenes/
│   └── game.c / .h             # Scene coordinator + tilemap parser
├── scene.h                     # Scene interface (Update/Render fn ptrs)
├── main.c                      # Entry point — window + lifecycle
└── platformer                  # Compiled binary
```

---

## 🧠 Engine Philosophy

### Data-Driven Execution
Level maps and world objects are **never hardcoded**. The engine parses
plain-text `.txt` matrixes at runtime. swap a file, get a new level.

### Complete System Encapsulation
`game.c` is a pure executive controller. `Enemy` and `Box` entities own
their transformation loops, state flags and collision resolution
entirely. Coordination happens through **pointer injection**, not globals.

### Polymorphic Weapon Dispatch
No `if/else` chains in the main loop. Each weapon binds a `fireFunc`
callback that executes its own ballistic profile independently.

```c
// Weapon bound at load time — dispatched polymorphically at runtime
weapon.fireFunc(bullets, &bulletCount, origin, direction);
```

---

## 🗺️ Tilemap Token Dictionary

```
┌───────┬─────────────────────┬──────────────────────────────────────┐
│ Token │ Entity              │ Description                          │
├───────┼─────────────────────┼──────────────────────────────────────┤
│   .   │ Air Space           │ Passable buffer — no physics alloc   │
│   P   │ Player Spawn        │ Initial origin vector for the player │
│  # =  │ Solid Geometry      │ Static platforms and boundaries      │
│   B   │ Destructible Box    │ Stomp-bounce breakable obstacle      │
│   E   │ Patrol Enemy        │ Active hazard — stomp-kill or damage │
└───────┴─────────────────────┴──────────────────────────────────────┘
```

**Example matrix fragment:**

```
................
###.....E....###
........B.......
P...............
================
```

---

## ⚔️ Weapon Profiles

| Weapon        | Mechanic                                           | Input     |
|---------------|----------------------------------------------------|-----------|
| Semi-Auto     | Single high-speed linear shot per tap              | Press     |
| Shotgun       | Multi-projectile burst with fixed angular spread   | Press     |
| Full-Auto     | Continuous stream with vertical recoil simulation  | Hold      |
| Flamethrower  | Dense short-range particle cluster with decay      | Hold      |

Projectile lifecycle is managed by a centralized **bullet memory pool**
(`MAX_BULLETS`). Allocation and deallocation never touch the heap at runtime.

---

## Controls

```
  ┌───┐         ┌───┐
  │ W │  Jump   │ Q │  Swap weapon slot
  └───┘         └───┘
┌───┬───┐    ┌───────┐
│ A │ D │    │ Space │  Fire active weapon
└───┴───┘    └───────┘
  Move
```

---

## Build

Compiles against any standard C toolchain linked to Raylib.

```bash
gcc main.c scenes/game.c entities/player.c entities/enemy.c \
    entities/box.c entities/bullet.c \
    -o platformer -lraylib -lm
```

> **Dependencies:** [Raylib](https://www.raylib.com/) — available in most
> package managers (`pacman -S raylib`, `apt install libraylib-dev`, etc.)

---

## Memory Model

Instance counts are tracked via live index registers at runtime:

```
platformCount   boxCount   enemyCount
     ↓               ↓           ↓
[ deterministic bounds — no heap growth during gameplay ]
```

All live instance arrays are **statically bounded** to maintain predictable
memory footprints and eliminate leak vectors in long sessions.

---

## 📄 License

See [`LICENSE`](./LICENSE).
