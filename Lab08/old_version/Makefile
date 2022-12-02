all: client server


client: client.cpp
	g++ client.cpp -o client -static

server: server.cpp
	g++ server.cpp -o server -static

clean:
	rm -f server client test