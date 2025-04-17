#include "NetworkThread.hpp"
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <cstring>


void client_position(SharedCubeState *state, int sock){
    std::lock_guard<std::mutex> lock(state->mutex);
        auto& me = state->players[state->player_id];
        std::string msg = "MOVE " + std::to_string(state->player_id) + " " +
                                    std::to_string((int)me.x) + " " +
                                    std::to_string((int)me.y) + "\n";
        send(sock, msg.c_str(), msg.size(), 0);
}

void run_network_thread(SharedCubeState *state, const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }
    
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Failed to connect to server\n";
        close(sock);
        return;
    }
    
    std::cout << "Connected to server at " << ip << ":" << port << std::endl;
    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, 1024);
    
    if (bytes_read > 0 && strncmp(buffer, "ID ", 3) == 0) {
        int assigned_id = buffer[3] - '0';
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->player_id = assigned_id;
            
            state->players[0].x = 100.f;
            state->players[0].y = 100.f;
            state->players[0].active = true;
            
            state->players[1].x = 650.f;
            state->players[1].y = 450.f;
            state->players[1].active = true;
        }
        
        std::cout << "Assigned Player ID: " << state->player_id << "\n";
    } else {
        std::cerr << "Failed to receive player ID\n";
        close(sock);
        return;
    }

    std::thread([sock, state]() {
        char recv_buf[1024];
        client_position(state, sock);        
        while (true) {
            client_position(state, sock);
            memset(recv_buf, 0, sizeof(recv_buf));
            int bytes = recv(sock, recv_buf, sizeof(recv_buf) - 1, MSG_DONTWAIT);
            
            if (bytes > 0) {
                recv_buf[bytes] = '\0';
                
                char* line = strtok(recv_buf, "\n");
                while (line != nullptr) {
                    int id, x, y;
                    if (sscanf(line, "MOVE %d %d %d", &id, &x, &y) == 3) {
                        if (id != state->player_id) {
                            std::lock_guard<std::mutex> lock(state->mutex);
                            state->players[id].x = x;
                            state->players[id].y = y;
                            state->players[id].active = true;
                        }
                    }
                    
                    line = strtok(nullptr, "\n");
                }
            }
            usleep(50000);
        }
    }
    ).detach();
}
