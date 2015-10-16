all: chatsrv client
charsrv: server.cpp
	g++ -std=c++11 -Wall -Werror -o chatsrv server.cpp
client: client.cpp
	g++ -std=c++11 -Wall -Werror -o client client.cpp
clean:
	rm -rf chatsrv
	rm -rf client
