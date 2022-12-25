CPP = g++
CPP_FLAG = -static -O3 -Ofast 
BROADCAST = 172.20.255.255

all: server client

server: receiver.cpp header.h
	$(CPP) receiver.cpp -o server $(CPP_FLAG)

client: sender.cpp header.h
	$(CPP) sender.cpp -o client $(CPP_FLAG)

submit: server client
	./submit.py ./server ./client

test: server client
	rm -f ./tmp/00*
	echo "=> sudo ./client ./test 4 $(BROADCAST)" 
	sudo ./server ./tmp 4 $(BROADCAST) 

clean:
	rm -f client server
	rm -f ./tmp/00*
