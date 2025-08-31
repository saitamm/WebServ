SRC =  server.cpp \
		srcs/Client/Client.cpp \
		srcs/Client/Request.cpp \
		srcs/Client/ParseRequest.cpp \
		srcs/Server/ConfigFile.cpp \
		srcs/Server/ParseConfigFile.cpp \
		srcs/Server/Methods/Get.cpp \
		srcs/Server/Methods/Delete.cpp \
		srcs/Server/Methods/Post.cpp \
		srcs/Server/Response.cpp \
		srcs/Server/ServerEngine.cpp \
		srcs/Server/ExecuteCgi.cpp \

NAME = WebServ

all: $(NAME)

CXX = c++
OBJ = $(SRC:.cpp=.o)
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g3  #-fsanitize=address
$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "\033[32mCompilation complete ✅\033[0m"

%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ)
fclean: clean 
	@rm -rf $(NAME)
	@echo "\033[31mExecutable removed ❌\033[0m"

re: fclean all