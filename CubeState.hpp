#pragma once
#include <mutex>

struct PlayerState {
    float x = 0.f;
    float y = 0.f;
    bool active = false;  // Whether this player is active in the game
};

struct SharedCubeState {
    PlayerState players[2];   // Players data (0 = Red, 1 = Blue)
    int player_id = -1;       // Which player this client controls (0 or 1)
    std::mutex mutex;
    
    // Debug function declaration
    void printState();
};