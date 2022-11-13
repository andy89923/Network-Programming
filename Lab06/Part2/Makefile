all: client server


client: client.cpp
	g++-7 client.cpp -o client -lpthread

server: server.cpp
	g++-7 server.cpp -o server

test: server
	./server 9998

clean:
	rm -f server client test