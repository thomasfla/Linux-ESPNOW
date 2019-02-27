test:
	mkdir -p bin
	gcc main.c -Wall if_helper.c -o bin/sender -pthread

debug:
	mkdir -p bin
	gcc main.c if_helper.c -g -Wall -o bin/sender -pthread
clean:
	rm -r bin
