TARGET=wwwd
OBJECT=server.cpp
CC=g++
CFLAGS= -std=c++11 -Wall -Werror -levent
WWWFLAGS= -h 127.0.0.1 -p 3010 -d /home/nickolas/Projects/OpenClasses_C/www/
all: $(TARGET)
$(TARGET): $(OBJECT)
	$(CC) -o $(TARGET) $(OBJECT) $(CFLAGS)
run:
	./$(TARGET) $(WWWFLAGS)
clean:
	rm -rf $(TARGET)
