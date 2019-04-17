
CC = g++
CFLAGS  = -g -Wall -std=c++11
INCLUDES = -I src/Includes/

default: all

%.o: src/%.cpp
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

app: main.o ESPNOW_manager.o ESPNOW_types.o
	$(CC) $(CFLAGS) -o bin/exec $(addprefix bin/,$^) -pthread


all: clear clean app

clean: 
	$(RM) bin/*

clear:
	clear
