TARGET=proxy
CFILES = proxy.cpp
CC=g++
CFLAGS = -std=c++11 -lev -Wall -Werror

all: $(TARGET)
$(TARGET): $(CFILES)
	$(CC) -o $(TARGET) $(CFILES) $(CFLAGS)
clean:
	rm -rf $(TARGET) *.tmp
