# PLC

Repository for SUTD Term 6 50.051 Programming Language Concept (PLC) 2026.

Project: **Abyss Walker** — a terminal maze game written in strict C90.

## Requirements

- `gcc`
- GNU `make`
  - **Linux:** `make` (pre-installed on most distros, or via your package manager)
  - **Windows:** `make` if using MSYS2, or `mingw32-make` if using a standalone MinGW-w64 install

## Build

From the project root:

**Linux**

```
make
```

**Windows (MSYS2)**

```
make
```

**Windows (standalone MinGW-w64)**

```
mingw32-make
```

The Makefile auto-detects the platform and produces `abyss_walker` on Linux and `abyss_walker.exe` on Windows. It compiles with `-Wall -Werror -ansi -pedantic` (strict C90).

### Manual build

If you prefer to invoke `gcc` directly:

On Linux:

```
gcc -Wall -Werror -ansi -pedantic main.c map.c enemy.c config.c save.c -o abyss_walker
```

On Windows:

```
gcc -Wall -Werror -ansi -pedantic main.c map.c enemy.c config.c save.c -o abyss_walker.exe
```

## Run

**Linux**

```
./abyss_walker
```

**Windows**

```
.\abyss_walker.exe
```

## Docker

Start the game with Docker Compose:

```bash
docker compose up --build abyss-walker
```

Stop and remove containers/networks after you exit:

```bash
docker compose down
```

## Configuration

The game reads its settings from `config.txt` using a `key=value` format.   
The supported keys are `width`, `height`, `seed`, and `density`.   
- `width` must be between `3` and `30`,
- `height` must be between `3` and `20`,
- `seed` must be `0` or greater,
- `density` must be between `0` and `100`.

A `seed` value of `0` uses the current time for random maze generation, while any other non-negative integer produces a fixed maze layout for repeatable runs.  
A `density` of `100` keeps the maze as a perfect maze with no extra openings, while lower values remove more internal walls and create a more open map.


## Controls

- `w` / `a` / `s` / `d` — move
- `f` — fire (requires picking up the bow `B`)
- `v` or `save` — save to `maze.sav`
- `l` or `load` — load from `maze.sav`
- `q` or `quit` — quit

## Clean

```
make clean
```

(or `mingw32-make clean` on standalone MinGW-w64).

## Cross-platform notes

- Source code is strict C90, no platform-specific headers or APIs.
- Save files (`maze.sav`) use explicit little-endian binary serialization, so a save written on one platform loads correctly on the other.
- Docker image uses a multi-stage build: compile in `gcc:14-bookworm`, run in `debian:bookworm-slim`.
