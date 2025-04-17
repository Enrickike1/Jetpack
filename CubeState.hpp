#pragma once
#include <mutex>

struct PlayerState {
    float x = 0.f;
    float y = 0.f;
    bool active = false;
};

struct SharedCubeState {
    PlayerState players[2];
    int player_id = -1;
    std::mutex mutex;

    void printState();
};