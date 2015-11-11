#include "my_inet.h"

int
set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

int
create_master_socket(char *ip, int port)
{
    //int port = 3010;
    int master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    set_nonblock(master_socket);

    struct sockaddr_in sock_adr;
    memset(&sock_adr, 0, sizeof(sock_adr));
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(port);
    //sock_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(ip, &sock_adr.sin_addr);
    int optval = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                   &optval, sizeof(optval)) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    if (bind(master_socket, (struct sockaddr *) &sock_adr,
             sizeof(sock_adr)) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    if (listen(master_socket, 0) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    return master_socket;
}

/*
* config format:
* <port_number>;<space><ip><space>:<port>[,<space><ip><space>:<port>]
*/
int
read_config(FILE *config)
{
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), config) == NULL)
        return -1;
    int port;
    int offset = 0;
    if (sscanf(buffer, "%d; %n", &port, &offset) != 1)
    {
        std::cout << "WARNING: empty line in config or"
                     " invalid port number\n";
        return -1;
    }
    int master_socket = create_master_socket(localhost, port);
    if (master_socket < 0)
    {
        std::cout << "ERROR: cannot create master_socket\n";
        return -1;
    }

    config_map[master_socket] = std::vector<std::pair<char*, int> >();
    char ip_local[20];
    char *tmp_pointer = buffer + offset;
    while (sscanf(tmp_pointer, "%s : %d%n", 
                  ip_local, &port, &offset) == 2)
    {
        tmp_pointer += offset + 1; // +1 for ','
        char *ip = strdup(ip_local);
        config_map[master_socket].push_back(std::make_pair(ip, port));
    }
    return master_socket;
}

void
print_config()
{
    for (auto i = config_map.begin(); i != config_map.end(); ++i)
    {
        printf("%d\n", i->first);
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            printf("\t%s:%d\n", i->second[j].first,
                   i->second[j].second);
        }
    }
}

int
create_client_socket(int master_socket)
{
    int client_socket = accept(master_socket, 0, 0);
    if (client_socket >= 0)
        set_nonblock(client_socket);
    return client_socket;
}

int
create_server_socket(char *ip, int port)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        return -1;
    set_nonblock(server_socket); // ?
    struct sockaddr_in sock_adr;
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(port);
    inet_aton(ip, &sock_adr.sin_adr);
    if (connect(server_socket, (struct sockaddr*)&sock_adr,
            sizeof(sock_adr) < 0))
    {
        close(server_socket);
        return -1;
    }
    return server_socket;
}
