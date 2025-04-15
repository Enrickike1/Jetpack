#pragma once
#include "CubeState.hpp"
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <cstring>

void run_network_thread(SharedCubeState* state, const char* ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Failed to connect\n";
        return;
    }

    char buffer[1024] = {0};
    read(sock, buffer, 1024);

    if (strncmp(buffer, "ID ", 3) == 0) {
        state->player_id = buffer[3] - '0';
        std::cout << "You are Player " << state->player_id << "\n";
        
        // Initialize player positions to different starting points
        std::lock_guard<std::mutex> lock(state->mutex);
        if (state->player_id == 0) {
            state->players[0].x = 100.f;
            state->players[0].y = 100.f;
            state->players[1].x = 650.f;
            state->players[1].y = 450.f;
        } else {
            state->players[0].x = 100.f;
            state->players[0].y = 100.f;
            state->players[1].x = 650.f;
            state->players[1].y = 450.f;
        }
    }

    std::thread([sock, state]() {
        while (true) {
            // Send our position to server
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                if (state->player_id >= 0) { // Only send if we have a valid ID
                    auto& me = state->players[state->player_id];
                    std::string msg = "MOVE " + std::to_string(state->player_id) + " " +
                                    std::to_string((int)me.x) + " " +
                                    std::to_string((int)me.y) + "\n";
                    send(sock, msg.c_str(), msg.size(), 0);
                }
            }

            // Process any incoming messages
            char recv_buf[1024] = {0};
            int bytes = recv(sock, recv_buf, sizeof(recv_buf) - 1, MSG_DONTWAIT);
            if (bytes > 0) {
                recv_buf[bytes] = '\0'; // Ensure null termination
                
                // Process all lines in the buffer
                char* line = strtok(recv_buf, "\n");
                while (line != nullptr) {
                    int id, x, y;
                    if (sscanf(line, "MOVE %d %d %d", &id, &x, &y) == 3) {
                        std::lock_guard<std::mutex> lock(state->mutex);
                        if (id >= 0 && id < 2 && id != state->player_id) {
                            // Only update position of OTHER players
                            state->players[id].x = x;
                            state->players[id].y = y;
                            std::cout << "Updated player " << id << " position to " << x << "," << y << std::endl;
                        }
                    }
                    line = strtok(nullptr, "\n");
                }
            }

            usleep(50000); // every 50ms
        }
    }).detach();
}