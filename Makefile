all: bin/client bin/server

bin/client: bin/client.o bin/common.o
	gcc -Wall bin/client.o bin/common.o -o client -lm

bin/server: bin/server.o bin/common.o
	gcc -Wall -lpthread bin/server.o bin/common.o -o server -lm

bin/client.o: client.c common.h
	gcc -Wall -c client.c -o bin/client.o

bin/server.o: server.c common.h
	gcc -Wall -c server.c -o bin/server.o

bin/common.o: common.c common.h
	gcc -Wall -c common.c -o bin/common.o

clean:
	rm -rf bin/*.o client server
