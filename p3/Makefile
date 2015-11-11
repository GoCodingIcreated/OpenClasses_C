TARGET=proxy
CFILES = proxy.cpp my_reciever.c my_inet.c
CC=g++
CFLAGS = -g -std=c++11 -lev -Wall -Werror

all: $(TARGET)
$(TARGET): $(CFILES)
	$(CC) -o $(TARGET) $(CFILES) $(CFLAGS)
clean:
	rm -rf $(TARGET) *.tmp
