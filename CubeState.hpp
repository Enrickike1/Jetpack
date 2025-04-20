#pragma once
#include <mutex>
#include <vector>

struct CoinState {
    float x, y;
    bool active;
};

struct PlayerState {
    float x = 0.f;
    float y = 0.f;
    bool active = false;
};

struct PlayerInput {
    bool jump = false;
};

struct SharedCubeState {
    PlayerState players[2];
    int player_id = -1;
    std::mutex mutex;

    void printState();
    PlayerInput input;
    std::vector<CoinState> coins;
};