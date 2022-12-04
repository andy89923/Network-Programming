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
	sudo tc qdisc add dev lo root netem delay 100ms 50ms loss 40% corrupt 10% rate 100Mbit
	tc qdisc show
	./server /tmp/tmp 4 12345 


clean:
	sudo tc qdisc del dev lo root netem
	rm -f client server
	rm -f /tmp/tmp/00*
