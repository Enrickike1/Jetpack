#pragma once
#include "CubeState.hpp"

// Function to run the network communication thread
void run_network_thread(SharedCubeState* state, const char* ip, int port);