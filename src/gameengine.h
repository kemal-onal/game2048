#pragma once

#include <stack>
#include <vector>

// Configurable game constants.
constexpr int N = 4;      // Board row count.
constexpr int M = 4;      // Board column count.
constexpr int K = 2048;   // Target tile value.
constexpr int P = 90;     // Chance of spawning a 2 (%).
constexpr int Q = 10;     // Chance of spawning a 4 (%).

static_assert(N > 0 && M > 0, "Board dimensions must be positive.");
static_assert(K > 0, "Target tile value must be positive.");
static_assert(P >= 0 && Q >= 0 && P + Q == 100, "P and Q must be non-negative and sum to 100.");

constexpr int BOARD_N = N;
constexpr int BOARD_M = M;
constexpr int TARGET_SCORE = K;
constexpr int CHANCE_P = P;
constexpr int CHANCE_Q = Q;

enum class Direction { UP, DOWN, LEFT, RIGHT };
enum class GameMode { NORMAL, UNLIMITED, HARD };
enum class GameState { PLAYING, WON, LOST };

// State saved before each valid move.
struct GameStateData {
    std::vector<std::vector<int>> board;
    int score;
};

class GameEngine {
public:
    GameEngine();

    void restart();
    bool slide(Direction dir); // Returns true if the move changes the board.
    void undo(); // Restores the previous saved board and score.

    // Read-only access for the UI.
    int getTile(int r, int c) const;
    int getScore() const;
    int getBestScore() const;
    GameState getState() const;
    GameMode getMode() const;

    void setMode(GameMode mode);

private:
    std::vector<std::vector<int>> board;
    std::stack<GameStateData> history; // Stores all previous valid states.

    int score;
    int bestScore;
    GameMode currentMode;
    GameState currentState;

    void spawnTile(int forcedValue = 0); // Adds a tile to a random empty cell.
    bool canMove() const; // Checks whether any valid move remains.
    void checkGameStatus(); // Updates win or loss state.

    // Slides and merges a single row or column.
    bool slideArray(std::vector<int>& line);
};
