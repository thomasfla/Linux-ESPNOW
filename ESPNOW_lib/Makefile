
CC = g++
CFLAGS  = -g -Wall
INCLUDES = -I src/Includes/

default: all

%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

app: main.o ESPNOW_manager.o ESPNOW_types.o
	$(CC) $(CFLAGS) -o bin/exec $(addprefix bin/,$^) -pthread


all: clear clean app

clean: 
	$(RM) bin/*

clear:
	clear
