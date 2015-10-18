#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>


enum
{
    PORT_NUMBER = 3010,
    TCP = 6
};

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

int main(int argc, char **argv)
{
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in sock_adr;
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(PORT_NUMBER);
    sock_adr.sin_addr.s_addr = htons(INADDR_ANY);
    int optval = 1;
    setsockopt(client_socket, TCP, SO_REUSEADDR,
               &optval, sizeof(optval));
    if (connect(client_socket, (struct sockaddr *) &sock_adr,
            sizeof(sock_adr)) < 0)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }
    struct pollfd pfd;
    //pfd[0].fd = 0; // stdin
    //pfd[0].events = POLLIN;
    pfd.fd = client_socket;
    pfd.events = POLLIN;
    char buffer[1024];
    while(1)
    {
        poll(&pfd, 1, 1);
        
        if (pfd.revents & POLLIN)
        {
            int readed = 0;
            readed = recv(pfd.fd, buffer, 
                          sizeof(buffer), MSG_NOSIGNAL);
            buffer[readed] = 0;
            printf("%s", buffer);
            fflush(stdout);
        }
        fgets(buffer, sizeof(buffer), stdin);
        if (strcmp(buffer, "EXIT\n") == 0)
            break;
        if (strcmp(buffer, "\n") == 0)
            continue;
        send(pfd.fd, buffer, strlen(buffer), MSG_NOSIGNAL);
    }
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    return 0;
}
