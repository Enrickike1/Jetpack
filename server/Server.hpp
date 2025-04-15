#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include <map>
#include <string>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

class Server {
public:
    Server(int port, const std::string& map_path);
    ~Server();
    bool run();
    void broadcast(const std::string& message);
    void send_to_client(int client_fd, const std::string& message);
    void handle_client_input(int fd);
    void load_map(const std::string& filepath);
    void start_game_if_ready();
    bool accept_cli();
    void run_game_session(int fd1, int fd2);  // Add this line


private:
    int server_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    int nfds;
    std::map<int, std::string> player_states;
    std::string map_data;
    bool game_started = false;
};

#endif
