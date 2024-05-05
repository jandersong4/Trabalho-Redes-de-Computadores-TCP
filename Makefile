all:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client -lm
	gcc -Wall server.c common.o -o server -lm
	gcc -Wall -lpthread server.c common.o -o server -lm
