#pragma once
#include <mutex>
#include <iostream>

struct PlayerState {
    float x = 0.f;
    float y = 0.f;
    bool active = false;  // Whether this player is active in the game
};

struct SharedCubeState {
    PlayerState players[2];   // Players data (0 = Red, 1 = Blue)
    int player_id = -1;       // Which player this client controls (0 or 1)
    std::mutex mutex;
    
    // Debug function
    void printState() {
        std::cout << "Player ID: " << player_id << std::endl;
        std::cout << "Player 0: (" << players[0].x << ", " << players[0].y << ") Active: " << players[0].active << std::endl;
        std::cout << "Player 1: (" << players[1].x << ", " << players[1].y << ") Active: " << players[1].active << std::endl;
    }
};