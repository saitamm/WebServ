SRC =  server.cpp \
		Request.cpp
NAME = WebServ

all: $(NAME)

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra --std=c++98 -g3  -fsanitize=address

$(NAME): $(SRC)
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(NAME)
	@echo "\033[32mCompilation complete ✅\033[0m"

fclean:
	@rm -rf $(NAME)
	@echo "\033[31mExecutable removed ❌\033[0m"

re: fclean all