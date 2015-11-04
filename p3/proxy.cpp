#include <stdio.h>
#include <iostream>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <ev.h>

#include <map>
#include <vector>
std::map<int, std::vector<std::pair<char*, int> > > config_map;
    
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

static void
read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[1024];
    int readed = recv(watcher->fd, buffer, sizeof(buffer),
                      MSG_NOSIGNAL);
    if (readed <= 0)
    {
        ev_io_stop(loop, watcher);
        free(watcher);
        return;
    }
    buffer[readed] = 0;
    printf("%s", buffer);
}

static void
accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    std::cout << "accept!\n";
    
    // client --> proxy --> server
    struct ev_io *client_watcher = (struct ev_io *) calloc(1,
                                    sizeof(*client_watcher));
    int client_socket = accept(watcher->fd, 0, 0);
    if (client_socket < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return;
    }
    set_nonblock(client_socket);
    ev_io_init(client_watcher, read_cb, client_socket, EV_READ);
    ev_io_start(loop, client_watcher);
/*
    // server --> proxy --> client
    struct ev_io *server_watcher = (struct ev_io *) calloc(1,
                                    sizeof(*server_watcher));
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return;
    }
    set_nonblock(server_socket);
    ev_io_init(client_watcher, read_cb, server_socket, EV_READ);
    ev_io_start(loop, server_watcher);
*/
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

void
read_config(FILE *config)
{
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), config) != NULL)
    { 
        int port;
        int offset = 0;
        if (sscanf(buffer, "%d%n", &port, &offset) == 2)
        {
            std::cout << "WARNING: empty line in config or"
                         " invalid port number\n";
            continue;
        }
        int master_socket = port;//cnt++;//create_socket(ip, port);
        config_map[master_socket] = std::vector<std::pair<char*, int> >();
        char ip_local[20];
        char *tmp_pointer = buffer + offset;
        while (sscanf(tmp_pointer, "%s:%d%n", 
                      ip_local, &port, &offset) == 2)
        {
            tmp_pointer += offset;
            char *ip = strdup(ip_local);
            config_map[master_socket].push_back(std::make_pair(ip, port));
        }
    }
}

void
print_config()
{
    for (auto i = config_map.begin(); i != config_map.end(); ++i)
    {
        printf("%d\n", i->first);
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            printf("\t!%s!:!%d!\n", i->second[j].first,
                   i->second[j].second);
        }
    }
}

int
main(int argc, char ** argv)
{
    FILE *config = NULL;
    if (argc < 2 || NULL == (config = fopen(argv[1], "r")))
    {
        std::cout << "No such config file\n";
        return 1;
    }
    read_config(config);
    print_config();
    fclose(config);
    return 0;
    
    struct ev_loop *loop = ev_default_loop(0);
    ev_io accept_watcher;
    int master_socket = 1;
    ev_io_init(&accept_watcher, accept_cb, master_socket, EV_READ);
    
    ev_io_start(loop, &accept_watcher);

    ev_run(loop);
    

    return 0;
}
