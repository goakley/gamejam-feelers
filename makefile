P=game
CFLAGS= -Wall -Werror -std=c99 -O0 -D DEBUG
LDLIBS = -lSDL -lpthread -lalut -lopenal -lm -lSDL_ttf
CC=gcc

$(P): $(OBJECTS)

valgrind : $(P)
	valgrind --leak-check=yes ./game

clean :
	find . -name game -exec rm {} \;
	find . -name "*~" -exec rm {} \;
