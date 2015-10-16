#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <set>

enum
{
    PORT_NUMBER = 3010,
    MAX_EVENTS = 32,
    TCP = 6
};

const char HELLO_MSG[] = "Welcome\n";


int set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}


int main(void)
{
    int master_socket;
    master_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (master_socket < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }
    struct sockaddr_in sock_adr;
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(PORT_NUMBER);
    sock_adr.sin_addr.s_addr = htons(INADDR_ANY);
    int optval = 1;
    setsockopt(master_socket, TCP, SO_REUSEADDR,
               &optval, sizeof(optval));
    if (bind(master_socket, (struct sockaddr *) &sock_adr,  
         sizeof(sock_adr)) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }
    if (listen(master_socket, 0) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }
    struct epoll_event event;
    event.data.fd = master_socket;
    event.events = EPOLLIN | EPOLLET;
    struct epoll_event *events;
    events = (struct epoll_event*) calloc(MAX_EVENTS, 
                                           sizeof(*events));
    int e_poll = epoll_create1(0);
    epoll_ctl(e_poll, EPOLL_CTL_ADD, master_socket, &event);
    std::set<int> clients;
    while (1)
    {
        size_t N = epoll_wait(e_poll, events, MAX_EVENTS, -1);
        for (size_t i = 0; i < N; ++i)
        {
            if ((events[i].events & EPOLLERR) || 
                (events[i].events & EPOLLHUP))
            {
                shutdown(events[i].data.fd, SHUT_RDWR);
                close(events[i].data.fd);
                fprintf(stdout, "connection terminated\n");
                fflush(stdout);
                clients.erase(events[i].data.fd);
            }
            else if(events[i].data.fd == master_socket)
            {
                int slave_socket = accept(master_socket, 0, 0);
                set_nonblock(slave_socket);
                struct epoll_event event;
                event.data.fd = slave_socket;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(e_poll, EPOLL_CTL_ADD, slave_socket, &event);
                fprintf(stdout, "accepted connection\n");
                fflush(stdout);
                clients.insert(slave_socket);
                send(slave_socket, HELLO_MSG, 
                     sizeof(HELLO_MSG), MSG_NOSIGNAL);
            }
            else
            {
                static char buffer[1024];
                int readed = recv(events[i].data.fd, buffer, 
                                  sizeof(buffer), MSG_NOSIGNAL);
                if (readed <= 0)
                {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    clients.erase(events[i].data.fd);
                    fprintf(stdout, "connection terminated\n");
                    fflush(stdout);
                    continue;
                }
                buffer[readed] = 0;
                fprintf(stdout, "%s", buffer);
                fflush(stdout);
                //std::cout << buffer << std::endl;
                for (auto it = clients.begin(); it != clients.end();
                     it++)
                {
                    send(*it, buffer,
                         readed, MSG_NOSIGNAL);
                }
            }
        }
    }
    shutdown(master_socket, SHUT_RDWR);
    close(master_socket);
    return 0;
}
