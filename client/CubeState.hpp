#pragma once
#include <mutex>

struct PlayerState {
    float x = 100.f;
    float y = 100.f;
};

struct SharedCubeState {
    PlayerState players[2];   // You and opponent
    int player_id = -1;       // 0 or 1 (assigned by server)
    std::mutex mutex;
};
