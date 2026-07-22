# 🎮 2048 Game Engine & UI (C++17 / Qt6)

A complete, object-oriented implementation of the classic 2048 puzzle game built with modern **C++17** and the **Qt6 Widgets** framework. This project demonstrates a strong separation of concerns, dividing core game logic from the graphical user interface, and features an automated CMake build system.

## 🏗️ Architecture & Tech Stack

* **Core Language:** C++17 (utilizing `std::vector`, `std::stack`, `std::random_device`, and `<algorithm>`)
* **GUI Framework:** Qt6 (Widgets, Event Handling, QTimer, Custom Stylesheets)
* **Build System:** CMake (Automoc, GNUInstallDirs)
* **Design Paradigm:** Strict Model-View separation. The `GameEngine` class acts as the pure data model and game state manager, completely independent of the UI, while `MainWindow` handles rendering, stylesheets, and input events.

## ✨ Key Features & Mechanics

* **Advanced State Management (Undo System):** Implements an infinite Undo feature using `std::stack<GameStateData>`, allowing players to revert to any previous board state, score, and tile configuration safely.
* **Dynamic Game Modes:**
  * **Normal Mode:** Classic rules. Reach the 2048 tile to win.
  * **Unlimited Mode:** Continue merging tiles past 2048 to achieve the highest possible score.
  * **Hard Mode:** Integrates asynchronous event handling via `QTimer`. If the player doesn't make a move within 5 seconds, the engine automatically forces a random valid slide.
* **Responsive & Adaptive UI:** Overrides Qt's `resizeEvent` to dynamically calculate window dimensions and layout spacing, ensuring the game board and tiles remain perfectly square and centered regardless of window resizing.
* **Configurable Engine Parameters:** Core mechanics (board dimensions N x M, spawn probabilities for 2s and 4s, target win score) are defined as `constexpr` in the header, making the engine highly modular and adaptable without rewriting core logic.

## 🚀 Building & Installation

### Prerequisites
* C++17 compatible compiler (GCC, Clang, or MSVC)
* CMake 3.21+
* Qt6 (specifically the Widgets component)

### Build Instructions

Clone the repository and compile using CMake:

    git clone [https://github.com/kemal-onal/game2048.git](https://github.com/kemal-onal/game2048.git)
    cd game2048
    cmake -S . -B build
    cmake --build build

Run the compiled executable:

    ./build/2048

*(Note: On Windows, the executable may be generated as `2048.exe` depending on the compiler and generator).*

Alternatively, use the included verification script:

    ./scripts/verify_build.sh

## ⌨️ Controls

* **W, A, S, D** or **Arrow Keys**: Slide tiles
* **U**: Undo last move
* **R**: Restart game
