NAME = webserv
SRC = main.cpp
INC = conf.hpp
OBJDIR = obj

CC=c++
FLAGS = -Wall -Wextra -Werror

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
