CPP=g++-7
CPP_FLAG=-static -O3 -Ofast 

all: server client

server: receiver.cpp header.h
	$(CPP) receiver.cpp -o server $(CPP_FLAG)

client: sender.cpp header.h
	$(CPP) sender.cpp -o client $(CPP_FLAG)

submit: server client
	./submit.py ./server ./client

test: server client
	rm -f /tmp/tmp/00*
	echo "=> ./client /tmp/test 4 12345 127.0.0.1"
	./server /tmp/tmp 4 12345 

clean:
	rm -f client server
	rm -f /tmp/tmp/00*
