NAME = webserv

SRCS_DIR = src/
BUILD_DIR = build/

SRCS_MAIN = main.cpp

SRCS_SERVER = Location.cpp Config.cpp 

SRCS_MESSAGES = AMessage.cpp AStartLine.cpp Header.cpp ResponseMessage.cpp RequestLine.cpp RequestMessage.cpp StatusLine.cpp

SRCS_UTILS = utilsSpace.cpp

SRCS := $(addprefix $(SRCS_DIR)main/, $(SRCS_MAIN)) \
		$(addprefix $(SRCS_DIR)server/, $(SRCS_SERVER)) \
		$(addprefix $(SRCS_DIR)messages/, $(SRCS_MESSAGES)) \
		$(addprefix $(SRCS_DIR)utils/, $(SRCS_UTILS)) \

OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.cpp=.o)))

DEP=$(OBJS:.o=.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./inc -MMD -MP

$(NAME): $(OBJS)
	@ echo " \033[33mCompiling webserv \033[m"
	@ $(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@ echo " \033[1;32m webserv compiled\033[m"

$(BUILD_DIR)%.o: $(SRCS_DIR)*/%.cpp
	@ mkdir -p $(BUILD_DIR)
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

clean:
	@ rm -f $(OBJS) $(DEP)
	@ rm -rd $(BUILD_DIR)
	@ echo " \033[32m Object files cleaned\033[m"

fclean: clean
	@ rm -f $(NAME)
	@ echo " \033[32m webserv cleaned\033[m"

re: fclean all

.PHONY: all fclean re clean

-include $(DEP)
