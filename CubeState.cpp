#include "CubeState.hpp"
#include <iostream>

void SharedCubeState::printState() {
    std::cout << "Player ID: " << player_id << std::endl;
    std::cout << "Player 0: (" << players[0].x << ", " << players[0].y << ") Active: " << players[0].active << std::endl;
    std::cout << "Player 1: (" << players[1].x << ", " << players[1].y << ") Active: " << players[1].active << std::endl;
}