CPP=g++
CPP_FLAG=-static

all: server client

server: server.cpp
	$(CPP) server.cpp -o server $(CPP_FLAG)

client: client.cpp
	$(CPP) client.cpp -o client $(CPP_FLAG)

test: server client
	rm -f /tmp/tmp/00*
	echo "=> ./client /tmp/test 4 12345 127.0.0.1"
	./server /tmp/tmp 4 12345 


clean:
	rm -f client server
	rm -f /tmp/tmp/00*