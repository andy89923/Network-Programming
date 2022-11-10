all: client server


client: client.cpp
	g++-7 client.cpp -o client

server: server.cpp
	g++-7 server.cpp -o server
	./server 10005

clean:
	rm -f server client