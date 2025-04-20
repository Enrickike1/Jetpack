#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <thread>
#include "../CubeState.hpp"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

class Server {
public:
    Server(int port, const std::string& map_path);
    ~Server();
    
    bool run();
    
private:
    int server_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds;
    std::string map_data;
    bool game_started = false;
    bool game_running = false;

    
    
    void load_map(const std::string& filepath);
    void send_to_client(int client_fd, const std::string& message);
    void broadcast(const std::string& message);
    void handle_client_input(int fd);
    void run_game_session(int fd1, int fd2);
    bool accept_cli();
    std::vector<CoinState> coins;
    void init_coins();
    void broadcast_coins();
    
    // New game logic methods
    void start_game_logic_thread();
    void update_game_state(float dt);
    int get_player_id_from_fd(int fd);
};