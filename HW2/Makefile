CPP = g++-7
CPP_FLAG = -O2 -std=c++11
FILE = server.cpp
INCLUDE = header.h handler.cpp dns.h

all: dns

dns: $(FILE) $(INCLUDE)
	$(CPP) $(FILE) -o dns $(CPP_FLAG)

test: dns
	./dns 12345 ./uploads/config.txt

clean:
	rm -f dns