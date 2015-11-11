#ifndef __MY_RECIEVER__
#define __MY_RECIEVR__

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

struct My_reciever;

typedef struct My_reciever My_reciever;

My_reciever* 
my_reciever_init(size_t _buffer_size,
                    int recv_fd, int sender_fd);
void
my_reciever_destroy(My_reciever *my_recv);

int
my_reciever_read(My_reciever *my_recv, size_t size);

int
my_reciever_write(My_reciever *my_recv, size_t size);

void
my_reciever_print(My_reciever *my_recv);

int
my_reciever_is_full(My_reciever *my_recv);

int
my_reciever_is_empty(My_reciever *my_recv);

int
my_reciever_get_reciever(My_reciever *my_recv);

void
my_reciever_close(My_reciever *my_recv);

int
my_reciever_is_closed(My_reciever *my_recv);

#endif
