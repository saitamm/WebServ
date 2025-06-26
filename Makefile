SRC =  server.cpp \
		cpp/Request.cpp \
		cpp/ConfigFile.cpp \
		cpp/ParseConfigFile.cpp 

NAME = WebServ

all: $(NAME)

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g3  

$(NAME): $(SRC)
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(NAME)
	@echo "\033[32mCompilation complete ✅\033[0m"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	@rm -rf $(OBJ)
fclean: clean 
	@rm -rf $(NAME)
	@echo "\033[31mExecutable removed ❌\033[0m"

re: fclean all