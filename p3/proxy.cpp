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
#include "my_reciever.h"
#include "my_inet.h"

std::map<int, std::vector<std::pair<char*, int> > > config_map;
std::map<int, My_reciever*> sender_reciever;
std::map<int, int> senders_map;

const int MAX_PORTS = 100;
const int MAX_BUFFER_SIZE = 1024;

static void
write_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    int fd = senders_map[watcher->fd];
    My_reciever *rec = sender_reciever[fd];
    if (rec == NULL)
    {
        printf("Hmmm, NULL pointer in write_cb\n");
        return;
    }
    int wrote = my_reciever_write(rec, MAX_BUFFER_SIZE);
    if (wrote == -1 || my_reciever_is_empty(rec))
    {
        senders_map.erase(watcher->fd);
        ev_io_stop(loop, watcher);
        free(watcher);
        if (wrote == -1 || my_reciever_is_closed(rec))
        {
            my_reciever_destroy(rec);
            sender_reciever[fd] = NULL;
            sender_reciever.erase(fd);
        }
    }
}

static void
read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    My_reciever *rec = sender_reciever[watcher->fd];
    if (rec == NULL)
    {
        printf("Hmmm, NULL pointer in read_cb\n");
        ev_io_stop(loop, watcher);
        free(watcher);
        sender_reciever.erase(watcher->fd);
        return;
    }
    int was_empty = my_reciever_is_empty(rec);
    if (my_reciever_is_full(rec))
        return;
    
    int readed = my_reciever_read(rec, MAX_BUFFER_SIZE);
    if (readed <= 0)
    {
        ev_io_stop(loop, watcher);
        free(watcher);
        my_reciever_close(rec);
        if (my_reciever_is_empty(rec))
        {
            my_reciever_destroy(rec);
            sender_reciever[watcher->fd] = NULL;
            sender_reciever.erase(watcher->fd);
        }
        return;
    }
    int reciever = my_reciever_get_reciever(rec);
    senders_map[reciever] = watcher->fd;
    if (was_empty)
    {
        struct ev_io *write_watcher = (struct ev_io *) 
            calloc(1, sizeof(*write_watcher));
        ev_io_init(write_watcher, write_cb, reciever, EV_WRITE);
        ev_io_start(loop, write_watcher);
    }
    //send(sender_reciever[watcher->fd], buffer, readed, MSG_NOSIGNAL);
}

static void
accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    std::cout << "accept!\n";
    
    // client --> proxy --> server
    int client_socket = create_client_socket(watcher->fd);
    if (client_socket < 0)
    {
        printf("Could not create client socket\n");
        return;
    }
    
    // server --> proxy --> client
    int rand_number = rand() % config_map[watcher->fd].size();
    auto tmp = config_map[watcher->fd][rand_number];
    int server_socket = create_server_socket(tmp.first, tmp.second);
    if (server_socket < 0)
    {
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        printf("Could not create server socket\n");
        return;
    }
    
    // client --> proxy --> server
    struct ev_io *client_watcher = (struct ev_io *) calloc(1,
                                    sizeof(*client_watcher));
    ev_io_init(client_watcher, read_cb, client_socket, EV_READ);
    ev_io_start(loop, client_watcher);
    
    // server --> proxy --> client
    struct ev_io *server_watcher = (struct ev_io *) calloc(1,
                                    sizeof(*server_watcher));
    ev_io_init(server_watcher, read_cb, server_socket, EV_READ);
    ev_io_start(loop, server_watcher);
    
    // save in map for every sender his adresat
    My_reciever *rec_serv, *rec_client;
    
    rec_client = my_reciever_init(MAX_BUFFER_SIZE, client_socket,
                                server_socket);
    rec_serv = my_reciever_init(MAX_BUFFER_SIZE, server_socket,
                                client_socket);
    sender_reciever[client_socket] = rec_serv;
    sender_reciever[server_socket] = rec_client;
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
    
    struct ev_loop *loop = ev_default_loop(0);
    ev_io accept_watchers[2 * MAX_PORTS]; // max listening ports
    int max_watcher = 0;
    int master_socket;
    while ((master_socket = read_config(config, config_map)) != -1)
    {
        if (max_watcher == 2 * MAX_PORTS)
            break;
        ev_io_init(&accept_watchers[max_watcher], accept_cb, 
                   master_socket, EV_READ);
        ev_io_start(loop, &accept_watchers[max_watcher]);
        max_watcher++;
    }
    print_config(config_map);
    fclose(config);
    
    ev_run(loop);
    

    return 0;
}
