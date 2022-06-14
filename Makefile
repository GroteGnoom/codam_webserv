NAME = webserv
SRC = main.cpp conf.cpp socket.cpp request.cpp response.cpp cgi.cpp body.cpp
INC = conf.hpp socket.hpp request.hpp response.hpp cgi.hpp
OBJDIR = obj

CC=c++
FLAGS = -Wall -Wextra -Werror -fsanitize=address

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

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: clean all
