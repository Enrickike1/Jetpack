CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -pthread

CLIENT_SRCS = client/main.cpp
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
CLIENT_BIN = client_app

SERVER_SRCS = server/Server.cpp server/main.cpp
SERVER_BIN = server_app

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRCS)
	$(CXX) $(CLIENT_SRCS) -o $(CLIENT_BIN) $(LDFLAGS)

$(SERVER_BIN): $(SERVER_SRCS)
	$(CXX) $(SERVER_SRCS) -o $(SERVER_BIN) -pthread

clean:
	rm -f $(CLIENT_BIN) $(SERVER_BIN)

fclean: clean

re: fclean all
