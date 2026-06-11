#include "gameengine.h"

#include <algorithm>
#include <random>

GameEngine::GameEngine()
    : score(0), bestScore(0), currentMode(GameMode::NORMAL), currentState(GameState::PLAYING) {
    restart();
}

void GameEngine::restart() {
    board.assign(BOARD_N, std::vector<int>(BOARD_M, 0));
    score = 0;
    currentState = GameState::PLAYING;

    // A new game should not keep old undo states.
    while (!history.empty()) {
        history.pop();
    }

    // The initial board starts with two 2 tiles.
    spawnTile(2);
    spawnTile(2);
}

void GameEngine::spawnTile(int forcedValue) {
    std::vector<std::pair<int, int>> emptyCells;

    // Collect empty cells first so the new tile position is chosen uniformly.
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            if (board[r][c] == 0) {
                emptyCells.emplace_back(r, c);
            }
        }
    }

    if (emptyCells.empty()) return;

    // Static generator keeps one random engine during the program run.
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, emptyCells.size() - 1);
    std::uniform_int_distribution<> chance(1, 100);

    auto [r, c] = emptyCells[dist(gen)];

    if (forcedValue != 0) {
        board[r][c] = forcedValue;
        return;
    }

    int roll = chance(gen);
    if (roll <= CHANCE_P) {
        board[r][c] = 2;
    } else if (roll <= CHANCE_P + CHANCE_Q) {
        board[r][c] = 4;
    }
}

bool GameEngine::slideArray(std::vector<int>& line) {
    bool moved = false;
    std::vector<int> newLine;
    newLine.reserve(line.size());

    // First pack all existing tiles toward the move direction.
    for (int val : line) {
        if (val != 0) newLine.push_back(val);
    }

    // Then merge equal neighbors from the border side.
    for (size_t i = 0; i + 1 < newLine.size(); ++i) {
        if (newLine[i] == newLine[i+1]) {
            newLine[i] *= 2;
            score += newLine[i];
            // Removing the second tile prevents a double merge in one move.
            newLine.erase(newLine.begin() + i + 1);
            moved = true;
        }
    }

    while (newLine.size() < line.size()) {
        newLine.push_back(0);
    }

    if (line != newLine) {
        line = newLine;
        moved = true;
    }

    return moved;
}

bool GameEngine::slide(Direction dir) {
    if (currentState != GameState::PLAYING) return false;

    // Save the old state before changing the board.
    GameStateData previousState = {board, score};
    bool anyMoved = false;

    // Rows and columns are converted to one line so the merge code is shared.
    if (dir == Direction::LEFT || dir == Direction::RIGHT) {
        for (int r = 0; r < BOARD_N; ++r) {
            std::vector<int> line = board[r];
            if (dir == Direction::RIGHT) std::reverse(line.begin(), line.end());

            if (slideArray(line)) anyMoved = true;

            if (dir == Direction::RIGHT) std::reverse(line.begin(), line.end());
            board[r] = line;
        }
    } else if (dir == Direction::UP || dir == Direction::DOWN) {
        for (int c = 0; c < BOARD_M; ++c) {
            std::vector<int> line(BOARD_N);
            for (int r = 0; r < BOARD_N; ++r) line[r] = board[r][c];

            if (dir == Direction::DOWN) std::reverse(line.begin(), line.end());

            if (slideArray(line)) anyMoved = true;

            if (dir == Direction::DOWN) std::reverse(line.begin(), line.end());
            for (int r = 0; r < BOARD_N; ++r) board[r][c] = line[r];
        }
    }

    // Only valid moves create history and a new tile.
    if (anyMoved) {
        history.push(previousState);
        spawnTile();
        if (score > bestScore) bestScore = score;
        checkGameStatus();
    }

    return anyMoved;
}

void GameEngine::undo() {
    if (!history.empty() && currentState == GameState::PLAYING) {
        // Undo restores score and board, but not the best score.
        board = history.top().board;
        score = history.top().score;
        history.pop();
    }
}

bool GameEngine::canMove() const {
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            if (board[r][c] == 0) return true;
        }
    }

    // A full board can still continue if equal neighbors exist.
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M - 1; ++c) {
            if (board[r][c] == board[r][c+1]) return true;
        }
    }

    for (int r = 0; r < BOARD_N - 1; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            if (board[r][c] == board[r+1][c]) return true;
        }
    }

    return false;
}

void GameEngine::checkGameStatus() {
    bool reachedTarget = false;

    // The target can appear anywhere on the board after a merge.
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            if (board[r][c] >= TARGET_SCORE) {
                reachedTarget = true;
            }
        }
    }

    if (reachedTarget && currentMode != GameMode::UNLIMITED) {
        currentState = GameState::WON;
    } else if (!canMove()) {
        currentState = GameState::LOST;
    }
}

int GameEngine::getTile(int r, int c) const { return board[r][c]; }
int GameEngine::getScore() const { return score; }
int GameEngine::getBestScore() const { return bestScore; }
GameState GameEngine::getState() const { return currentState; }
GameMode GameEngine::getMode() const { return currentMode; }

void GameEngine::setMode(GameMode mode) {
    currentMode = mode;
    // Winning can continue only when the player switches to Unlimited Mode.
    if (currentState == GameState::WON && mode == GameMode::UNLIMITED) {
        currentState = GameState::PLAYING;
    }
}
