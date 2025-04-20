#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <chrono>
#include <cmath>
#include <algorithm>

const float GRAVITY = 400.0f;
const float JUMP_POWER = 1000.0f;
const float MAX_FALL_SPEED = 400.0f;
const float WINDOW_WIDTH = 800.0f;
const float WINDOW_HEIGHT = 600.0f;
const float CUBE_SIZE = 50.0f;

struct ServerPlayerState {
    float x = 100.0f;
    float y = 100.0f;
    float velocity_y = 0.0f;
    bool active = false;
    bool jump_pressed = false;
};

ServerPlayerState players[2];

Server::Server(int port, const std::string& map_path) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    nfds = 1;

    players[0].x = 100.0f;
    players[0].y = 100.0f;
    players[1].x = 100.0f;
    players[1].y = 100.0f;
    
    load_map(map_path);
    std::cout << "Server started on port " << port << "\n";
    
    init_coins();
}

Server::~Server() {
    close(server_fd);
}

void Server::init_coins() {
    const int NUM_COINS = 5;
    for (int i = 0; i < NUM_COINS; i++) {
        CoinState coin;
        coin.x = 100 + i * 150;
        coin.y = 200;
        coin.active = true;
        coins.push_back(coin);
    }
    
    std::cout << "Initialized " << coins.size() << " coins" << std::endl;
    broadcast_coins();
}

void Server::broadcast_coins() {
    for (size_t i = 0; i < coins.size(); i++) {
        if (coins[i].active) {
            std::string msg = "COIN " + std::to_string(i) + " " + 
                            std::to_string(static_cast<int>(coins[i].x)) + " " + 
                            std::to_string(static_cast<int>(coins[i].y)) + "\n";
            broadcast(msg);
            std::cout << "Broadcasting coin: " << i << " at (" << coins[i].x << ", " << coins[i].y << ")" << std::endl;
        }
    }
}


void Server::load_map(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to load map from: " << filepath << "\n";
        exit(1);
    }

    map_data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void Server::send_to_client(int client_fd, const std::string& message) {
    send(client_fd, message.c_str(), message.size(), 0);
}

void Server::broadcast(const std::string& message) {
    for (int i = 1; i < nfds; i++) {
        send(fds[i].fd, message.c_str(), message.size(), 0);
    }
}

void Server::update_game_state(float dt) {
    for (int i = 0; i < 2; i++) {
        if (!players[i].active) continue;
        
        players[i].velocity_y += GRAVITY * dt;
        if (players[i].jump_pressed) {
            players[i].velocity_y -= JUMP_POWER * dt;
        }
        if (players[i].velocity_y > MAX_FALL_SPEED) {
            players[i].velocity_y = MAX_FALL_SPEED;
        }

        players[i].y += players[i].velocity_y * dt;

        if (players[i].y > WINDOW_HEIGHT - CUBE_SIZE) {
            players[i].y = WINDOW_HEIGHT - CUBE_SIZE;
            players[i].velocity_y = 0;
        }
        if (players[i].y < 0) {
            players[i].y = 0;
            players[i].velocity_y = 0;
        }

        players[i].x = std::max(0.0f, std::min(players[i].x, WINDOW_WIDTH - CUBE_SIZE));
    }
}

void Server::start_game_logic_thread() {
    std::thread([this]() {
        auto last_time = std::chrono::high_resolution_clock::now();
        int resync_counter = 0;

        while (true) {
            if (!game_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto current_time = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;

            if (dt > 0.1f) dt = 0.1f;

            update_game_state(dt);

            for (int i = 0; i < 2; i++) {
                if (players[i].active) {
                    std::string msg = "MOVE " + std::to_string(i) + " " + 
                                      std::to_string(static_cast<int>(players[i].x)) + " " + 
                                      std::to_string(static_cast<int>(players[i].y)) + "\n";
                    broadcast(msg);
                }
            }

            resync_counter++;
            if (resync_counter >= 180) {
                resync_counter = 0;
                broadcast_coins();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }).detach();
}

void Server::handle_client_input(int fd) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes <= 0) {
        std::cout << "Client " << fd << " disconnected\n";
        close(fd);
        
        for (int i = 1; i < nfds; ++i) {
            if (fds[i].fd == fd) {
                for (int j = i; j < nfds - 1; ++j) {
                    fds[j] = fds[j + 1];
                }
                nfds--;
                break;
            }
        }

        int player_id = get_player_id_from_fd(fd);
        if (player_id != -1) {
            players[player_id].active = false;
        }

        if (game_running && (!players[0].active || !players[1].active)) {
            std::cout << "A player disconnected. Stopping game." << std::endl;
            game_running = false;
        }
    } else {
        buffer[bytes] = '\0';
        std::string msg(buffer);

        int player_id;
        char cmd[256];
        if (sscanf(buffer, "INPUT %d", &player_id) == 1) {
            players[player_id].jump_pressed = false;

            if (strstr(buffer, "JUMP") != NULL) {
                players[player_id].jump_pressed = true;
            }
        }
    }
}



int Server::get_player_id_from_fd(int fd) {

    for (int i = 1; i < nfds; i++) {
        if (fds[i].fd == fd) {
            return (i - 1) % 2;
        }
    }
    return -1;
}

bool Server::run() {
    std::cout << "Server running and waiting for connections...\n";

    start_game_logic_thread();

    while (true) {
        int poll_result = poll(fds, nfds, 1000);

        if (poll_result < 0) {
            perror("poll failed");
            return false;
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

            if (new_client < 0) {
                perror("accept failed");
                continue;
            }

            if (nfds < MAX_CLIENTS) {
                fds[nfds].fd = new_client;
                fds[nfds].events = POLLIN;

                int player_id = (nfds - 1) % 2;
                std::string id_msg = "ID " + std::to_string(player_id) + "\n";
                send_to_client(new_client, id_msg);

                for (size_t i = 0; i < coins.size(); i++) {
                    if (coins[i].active) {
                        std::string msg = "COIN " + std::to_string(i) + " " + 
                                          std::to_string(static_cast<int>(coins[i].x)) + " " + 
                                          std::to_string(static_cast<int>(coins[i].y)) + "\n";
                        send_to_client(new_client, msg);
                    }
                }

                players[player_id].active = true;

                std::cout << "New client connected: FD=" << new_client << ", assigned Player ID " << player_id << std::endl;
                nfds++;

                if (players[0].active && players[1].active && !game_running) {
                    std::cout << "Both players connected. Starting game." << std::endl;
                    game_running = true;
                }
            } else {
                std::cout << "Too many clients, rejecting connection\n";
                close(new_client);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                handle_client_input(fds[i].fd);
            }
        }
    }

    return true;
}