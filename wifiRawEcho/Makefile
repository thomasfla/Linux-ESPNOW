
CC = gcc
CFLAGS  = -g -Wall --std=c99
INCLUDES = -I Includes/

default: all

%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

app: main.o ESPNOW_packet.o
	$(CC) $(CFLAGS) -o bin/exec $(addprefix bin/,$^)


all: clear clean app

clean: 
	$(RM) bin/*

clear:
	clear
