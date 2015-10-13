#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

enum
{
    PORT_NUMBER = 3010
};

int main(int argc, char **argv)
{
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct socketaddr_in sock_adr;
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(PORT_NUMBER);
    return 0;
}
