CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

SERVER_SOURCES = Server/main.cpp Server/Server.cpp Objects.cpp
SERVER_TARGET = jetpack_server

CLIENT_SOURCES = Client/main.cpp Client/Game.hpp Client/Game.cpp CubeState.cpp NetworkClient.cpp 
CLIENT_TARGET = jetpack_client
CLIENT_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all: server client

server: $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_SOURCES)

client: $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SOURCES) $(CLIENT_LIBS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)

.PHONY: all server client clean