#ifndef __MY_INET__
#define __MY_INET__

#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

int
set_nonblock(int fd);

int
create_master_socket(char *ip, int port);

/*
* config format:
* <port_number>;<space><ip><space>:<port>[,<space><ip><space>:<port>]
*/
int
read_config(FILE *config);

void
print_config();

int
create_client_socket(int master_socket);

int
create_server_socket(char *ip, int port);

#endif
