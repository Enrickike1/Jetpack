NAME = jetpack_server

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC = main.cpp Server.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
