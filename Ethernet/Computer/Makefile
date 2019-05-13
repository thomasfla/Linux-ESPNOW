
CC = gcc
CFLAGS  = -g -Wall -std=c99
INCLUDES = -I src/Includes/

default: all

%.o: src/%.c
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

app: main.o
	$(CC) $(CFLAGS) -o bin/exec $(addprefix bin/,$^) -pthread


all: clear clean app

clean: 
	$(RM) bin/*

clear:
	clear
