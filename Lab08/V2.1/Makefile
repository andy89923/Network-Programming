CPP=g++
CPP_FLAG=-static -O3 -Ofast 

all: server client

server: server.cpp header.h
	$(CPP) server.cpp -o server $(CPP_FLAG)

client: client.cpp header.h
	$(CPP) client.cpp -o client $(CPP_FLAG)

submit: server client
	./submit.py ./server ./client 9AQ853AEVK7NB6XH 

test: server client
	rm -f /tmp/tmp/00*
	echo "=> ./client /tmp/test 4 12345 127.0.0.1"
	tc qdisc show
	./server /tmp/tmp 4 12345 

add_rule:
	sudo tc qdisc add dev lo root netem delay 100ms 50ms loss 40% corrupt 10% rate 100Mbit

del_rule:
	sudo tc qdisc del dev lo root netem

clean:
	rm -f client server
	rm -f /tmp/tmp/00*
