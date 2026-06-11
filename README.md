# 2048

This repository contains a Qt Widgets implementation of the 2048 game. The game supports Normal, Unlimited, and Hard modes, score and best score tracking, Undo, Restart, and configurable board and tile-generation constants.

The main source files are under `src/`:

- `main.cpp`: starts the Qt application.
- `mainwindow.h/.cpp`: handles the user interface, keyboard input, buttons, and timer events.
- `gameengine.h/.cpp`: stores the board and implements movement, merging, scoring, undo, and win/loss logic.

## Build and Run

The project uses CMake and produces an executable named `2048`.

```bash
cmake -S . -B build
cmake --build build
./build/2048
```

On Windows, the executable may be generated as `2048.exe` depending on the compiler and generator.

You can also use the verification script:

```bash
./scripts/verify_build.sh
```

Generated build folders and compiled binaries should not be committed.
