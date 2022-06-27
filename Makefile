NAME = webserv
SRC = main.cpp conf.cpp socket.cpp request.cpp response.cpp cgi.cpp body.cpp post.cpp delete.cpp list_files.cpp
INC = conf.hpp socket.hpp request.hpp response.hpp cgi.hpp post.hpp delete.hpp list_files.hpp
OBJDIR = obj

CC=c++
FLAGS = -Wall -Wextra -Werror -fsanitize=address -g

OBJ = $(SRC:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp $(INC)
	$(CC) $(FLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

test: all
	cd tests && ./tests.sh

siege: all
	./webserv > output &
	sleep 1
	siege 127.0.0.1:8080 -t10s -b
	pkill webserv

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: clean all
