#ifndef __MY_INET__
#define __MY_INET__

#include <iostream>
#include <stdio.h>
#include <string.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <map>
#include <vector>

int
set_nonblock(int fd);

int
create_master_socket(char *ip, int port);

/*
* config format:
* <port_number>;<space><ip><space>:<port>[,<space><ip><space>:<port>]
*/
int
read_config(FILE *config, std::map<int,
        std::vector<std::pair<char*, int> > > &config_map);
void
print_config(std::map<int, 
            std::vector<std::pair<char*, int> > > &config_map);

int
create_client_socket(int master_socket);

int
create_server_socket(char *ip, int port);

#endif
