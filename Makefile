SRC =  server.cpp \
		Help.cpp \
		cpp/Request.cpp \
		cpp/ConfigFile.cpp \
		cpp/Client.cpp \
		cpp/ParseConfigFile.cpp \
		cpp/Response.cpp \
		Methods/Delete.cpp \
		Methods/Get.cpp \
		Methods/Post.cpp  

NAME = WebServ

all: $(NAME)

CXX = c++
OBJ = $(SRC:.cpp=.o)
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g3  #-fsanitize=address
$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "\033[32mCompilation complete ✅\033[0m"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	@rm -rf $(OBJ)
fclean: clean 
	@rm -rf $(NAME)
	@echo "\033[31mExecutable removed ❌\033[0m"

re: fclean all