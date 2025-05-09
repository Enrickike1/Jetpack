#include "NetworkClient.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

NetworkClient::NetworkClient(SharedCubeState* state) : 
    state(state), 
    sock(-1),
    running(false) {
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect(const char* ip, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (::connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Failed to connect to server\n";
        close(sock);
        sock = -1;
        return false;
    }
    
    std::cout << "Connected to server at " << ip << ":" << port << std::endl;
    
    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, 1024);
    
    if (bytes_read > 0 && strncmp(buffer, "ID ", 3) == 0) {
        int assigned_id = buffer[3] - '0';
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->player_id = assigned_id;

            state->players[0].active = true;
            state->players[1].active = true;
        }
        
        std::cout << "Assigned Player ID: " << assigned_id << "\n";
    } else {
        std::cerr << "Failed to receive player ID\n";
        close(sock);
        sock = -1;
        return false;
    }

    running = true;
    network_thread = std::thread(&NetworkClient::network_loop, this);
    
    return true;
}

void NetworkClient::disconnect() {
    if (running) {
        running = false;
        if (network_thread.joinable()) {
            network_thread.join();
        }
    }
    
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}

bool NetworkClient::is_connected() const {
    return sock >= 0 && running;
}

void NetworkClient::send_input_to_server() {
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->player_id == -1) return;

    std::string msg = "INPUT " + std::to_string(state->player_id) + " ";

    if (state->input.jump) msg += "JUMP ";

    msg += "\n";
    send(sock, msg.c_str(), msg.size(), 0);
}

void NetworkClient::process_server_messages() {
    char recv_buf[1024];
    memset(recv_buf, 0, sizeof(recv_buf));
    
    int bytes = recv(sock, recv_buf, sizeof(recv_buf) - 1, MSG_DONTWAIT);
    
    if (bytes > 0) {
        recv_buf[bytes] = '\0';
        
        char* line = strtok(recv_buf, "\n");
        while (line != nullptr) {
            int coin_id, coin_x, coin_y;
            // if (sscanf(line, "COIN %d %d %d", &coin_id, &coin_x, &coin_y) == 3) {
            //     std::cout << "Received coin: " << coin_id << " at (" << coin_x << ", " << coin_y << ")" << std::endl;
            //     std::lock_guard<std::mutex> lock(state->mutex);
            //     if (coin_id >= state->coins.size()) {
            //         state->coins.resize(coin_id + 1);
            //     }
            //     state->coins[coin_id].x = coin_x;
            //     state->coins[coin_id].y = coin_y;
            //     state->coins[coin_id].active = true;
            // }

            int id, x, y;
            if (sscanf(line, "MOVE %d %d %d", &id, &x, &y) == 3) {
                std::lock_guard<std::mutex> lock(state->mutex);
                if (id >= 0 && id < 2) {
                    state->players[id].x = x;
                    state->players[id].y = y;
                    state->players[id].active = true;
                }
            }
            
            line = strtok(nullptr, "\n");
        }
    }
}

void NetworkClient::network_loop() {
    while (running) {
        send_input_to_server();
        process_server_messages();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}