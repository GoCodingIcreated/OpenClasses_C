TARGET=allocator
SOURCE=allocator.c
OBJECTS=allocator.o
CFLAGS=-g -std=c++11 -Wall -Werror
CC=g++
all:$(TARGET)
$(TARGET):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
$(OBJECTS):$(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE)
clean:
	rm *.o allocator *.out
