CPP=g++-7
CPP_FLAG=-std=c++17
SOURCE=server.cpp error.cpp user.cpp handler.cpp channel.cpp
TEMPLATE_PATH=template

TEST_PROG=server

all: server

server: $(SOURCE)
	$(CPP) $(CPP_FLAG) -o $(TEST_PROG) $(SOURCE) -I $(TEMPLATE_PATH)

test: $(TEST_PROG)
	./$(TEST_PROG) 10005


clean:
	rm -f $(TEST_PROG)