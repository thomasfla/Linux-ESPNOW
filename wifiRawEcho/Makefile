
CC = gcc
CFLAGS  = -g -Wall
INCLUDES = -I Includes/

default: all

%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

app: main.o packets.o
	$(CC) $(CFLAGS) -o bin/exec $(addprefix bin/,$^)


all: clear clean app

clean: 
	$(RM) bin/*

clear:
	clear
