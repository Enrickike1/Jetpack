#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>

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

    load_map(map_path);
    std::cout << "Server started on port " << port << "\n";
}

Server::~Server() {
    close(server_fd);
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
    } else {
        buffer[bytes] = '\0';
        std::string msg(buffer);
        // std::cout << "Received from client " << fd << ": " << msg;
        
        int player_id, x, y;
        if (sscanf(buffer, "MOVE %d %d %d", &player_id, &x, &y) == 3) {
            for (int i = 1; i < nfds; i++) {
                send(fds[i].fd, msg.c_str(), msg.length(), 0);
            }
        }
    }
}

void Server::run_game_session(int fd1, int fd2) {
    send_to_client(fd1, "Game starting! You are Player 1\n" + map_data);
    send_to_client(fd2, "Game starting! You are Player 2\n" + map_data);

    char buffer[BUFFER_SIZE];
    while (true) {
        sleep(1);
    }
}

bool Server::accept_cli() {
    static int players_connected = 0;
    static int player_sockets[2] = { -1, -1 };

    while (true) {
        int ret = poll(fds, nfds, -1);
        if (ret == -1) {
            perror("poll");
            return false;
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (new_client_fd == -1) {
                perror("accept");
                continue;
            }

            fcntl(new_client_fd, F_SETFL, O_NONBLOCK);
            std::cout << "New client connected: FD=" << new_client_fd << std::endl;
            send_to_client(new_client_fd, "Welcome! Waiting for another player...\n");

            player_sockets[players_connected++] = new_client_fd;
            if (players_connected == 0) {
                send_to_client(new_client_fd, "ID 0\n");
            } else if (players_connected == 1) {
                send_to_client(new_client_fd, "ID 1\n");
            }
            
            if (players_connected == 2) {
                std::cout << "Two players connected. Starting game session...\n";

                pid_t pid = fork();
                if (pid == 0) {
                    run_game_session(player_sockets[0], player_sockets[1]);
                    close(player_sockets[0]);
                    close(player_sockets[1]);
                    exit(0);
                }

                players_connected = 0;
            }
        }
    }
    return true;
}

bool Server::run() {
    std::cout << "Server running and waiting for connections...\n";
    
    while (true) {
        int poll_result = poll(fds, nfds, 1000); // Poll with 1 second timeout
        
        if (poll_result < 0) {
            perror("poll failed");
            return false;
        }
        
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (new_client < 0) {
                perror("accept failed");
                continue;
            }
            
            // Add to poll set
            if (nfds < MAX_CLIENTS) {
                fds[nfds].fd = new_client;
                fds[nfds].events = POLLIN;
                
                // Assign player ID (0 or 1)
                int player_id = (nfds - 1) % 2;
                std::string id_msg = "ID " + std::to_string(player_id) + "\n";
                send_to_client(new_client, id_msg);
                
                std::cout << "New client connected: FD=" << new_client << ", assigned Player ID " << player_id << std::endl;
                nfds++;
            } else {
                std::cout << "Too many clients, rejecting connection\n";
                close(new_client);
            }
        }
        
        // Check existing connections for data
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                handle_client_input(fds[i].fd);
            }
        }
    }
    
    return true;
}