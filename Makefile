NAME = webserv

SRCS = main.cpp Server.cpp

OBJS = $(addprefix build/, $(SRCS:.cpp=.o))

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

$(NAME): $(OBJS)
	@ echo " \033[33mCompiling webserv \033[m"
	@ $(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@ echo " \033[1;32m webserv compiled\033[m"

build/%.o: %.cpp
	@ mkdir -p build/
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

clean:
	@ rm -f $(OBJS)
	@ rm -df build
	@ echo " \033[32m Object files cleaned\033[m"

fclean: clean
	@ rm -f $(NAME)
	@ echo " \033[32m webserv cleaned\033[m"

re: fclean all

.PHONY: all fclean re clean
