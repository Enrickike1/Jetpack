#pragma once
#include "CubeState.hpp"

void run_network_thread(SharedCubeState* state, const char* ip, int port);
void client_position(SharedCubeState *state, int sock);