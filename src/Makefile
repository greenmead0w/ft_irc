NAME        = ircserv
CC          = c++
FLAGS       = -Wall -Wextra -Werror -std=c++98 -I./include #to look headers inside the include folder

#Organize source files by category
SRC_DIR     = src/
OBJ_DIR     = obj/

#Add files here as we create them
FILES       = main.cpp Server.cpp Client.cpp Channel.cpp #more to be added 
COMMANDS    = PASS.cpp NICK.cpp USER.cpp JOIN.cpp #more to be added

#Constructing file paths
SRCS        = $(addprefix $(SRC_DIR), $(FILES)) \
              $(addprefix $(SRC_DIR)Commands/, $(COMMANDS))
OBJS        = $(SRCS:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)
	@echo "\033[32m[ft_irc] Server Compiled Successfully\033[0m"

#Ensure subdirectories exist in obj/ for Commands .cpp files
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
