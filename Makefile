NAME = webserv

SRCS_DIR = src/
HEADERS_DIR = inc/
BUILD_DIR = build/

SRCS_MAIN = main.cpp

SRCS_SERVER = Location.cpp Application.cpp Config.cpp Server.cpp
SRCS_MESSAGES = AMessage.cpp AStartLine.cpp CgiHandler.cpp Header.cpp ResponseMessage.cpp RequestLine.cpp \
				MethodHandler.cpp RequestMessage.cpp StatusLine.cpp RequestHandler.cpp CreateResponse.cpp

SRCS_UTILS = utilsSpace.cpp utilsParsing.cpp

SRCS := $(addprefix $(SRCS_DIR)main/, $(SRCS_MAIN)) \
		$(addprefix $(SRCS_DIR)server/, $(SRCS_SERVER)) \
		$(addprefix $(SRCS_DIR)messages/, $(SRCS_MESSAGES)) \
		$(addprefix $(SRCS_DIR)utils/, $(SRCS_UTILS)) \

OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.cpp=.o)))

DEP=$(OBJS:.o=.d)

CXX = c++

INCLUDE_FLAGS = -I$(HEADERS_DIR) -I$(HEADERS_DIR)server -I$(HEADERS_DIR)messages -I$(HEADERS_DIR)utils

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDE_FLAGS) -MMD -MP

$(NAME): $(OBJS)
	@ echo " \033[33mCompiling Webserv \033[m"
	@ $(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@ echo " \033[32m \033[1mWebserv\033[22m compiled\033[m"

$(BUILD_DIR)%.o: $(SRCS_DIR)*/%.cpp
	@ mkdir -p $(BUILD_DIR)
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

TEST_NAME = test_webserv
TEST_DIR = test/

$(TEST_DIR)%.o: $(TEST_DIR)%.cpp
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

TEST_SRCS := $(addprefix $(SRCS_DIR)server/, $(SRCS_SERVER)) \
			 $(addprefix $(SRCS_DIR)messages/, $(SRCS_MESSAGES)) \
		     $(addprefix $(SRCS_DIR)utils/, $(SRCS_UTILS)) \

TEST_OBJS := $(addprefix $(BUILD_DIR), $(notdir $(TEST_SRCS:.cpp=.o))) \
			 $(TEST_DIR)test.o

TEST_DEP=$(TEST_OBJS:.o=.d)

test: $(TEST_OBJS)
	@ echo " \033[33mCompiling Webserv tests\033[m"
	@ $(CXX) $(CXXFLAGS) -o $(TEST_NAME) $(TEST_OBJS)
	@ echo " \033[34m \033[1mWebserv tests\033[22m compiled\033[m"
	@ ./test_webserv

all: $(NAME)

clean:
	@ rm -f $(OBJS) $(DEP) $(TEST_OBJS) $(TEST_DEP)
	@ rm -rfd $(BUILD_DIR)
	@ echo " \033[32m Object files cleaned\033[m"

fclean: clean
	@ rm -f $(NAME) $(TEST_NAME)
	@ echo " \033[32m \033[1m Webserv\033[22m cleaned\033[m"

re: fclean all


.PHONY: all fclean re clean test

-include $(DEP)
