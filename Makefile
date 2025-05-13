NAME = webserv

SRCS_DIR = src/
BUILD_DIR = build/

SRCS = main.cpp

SRCS := $(addprefix $(SRCS_DIR), $(SRCS))

OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.cpp=.o)))

DEP=$(OBJS:.o=.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./inc -MMD -MP

$(NAME): $(OBJS)
	@ echo " \033[33mCompiling webserv \033[m"
	@ $(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@ echo " \033[1;32m webserv compiled\033[m"

$(BUILD_DIR)%.o: $(SRCS_DIR)%.cpp
	@ mkdir -p $(BUILD_DIR)
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

clean:
	@ rm -f $(OBJS) $(DEP)
	@ rm -df $(BUILD_DIR)
	@ echo " \033[32m Object files cleaned\033[m"

fclean: clean
	@ rm -f $(NAME)
	@ echo " \033[32m webserv cleaned\033[m"

re: fclean all

.PHONY: all fclean re clean

-include $(DEP)
