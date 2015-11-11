#include "my_reciever.h"

struct My_reciever
{
    char *buffer1, *buffer2;
    size_t buffer_size, data_size;
    int begin;
    int reciever_fd;
    int sender_fd;
    int closed;
};


My_reciever* 
my_reciever_init(size_t _buffer_size,
                    int recv_fd, int send_fd)
{
    My_reciever *my_recv = (My_reciever*) calloc(1, sizeof(*my_recv));
    my_recv->begin = 0;
    my_recv->buffer_size = _buffer_size;
    my_recv->data_size = 0;
    my_recv->reciever_fd = recv_fd;
    my_recv->sender_fd = send_fd;
    my_recv->buffer1 = (char*) calloc(_buffer_size, sizeof(char));
    my_recv->buffer2 = (char*) calloc(_buffer_size, sizeof(char));
    my_recv->closed = 0;
    return my_recv;
}

void
my_reciever_destroy(My_reciever *my_recv)
{
    free(my_recv->buffer1);
    free(my_recv->buffer2);
    free(my_recv);
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int
my_reciever_read(My_reciever *rcv, size_t size)
{
    int tmp = min(size, rcv->buffer_size - rcv->data_size);
    if (tmp == 0)
        return 0;
    int readed = recv(rcv->sender_fd, rcv->buffer2, tmp, MSG_NOSIGNAL);
    if (readed <= 0)
        return readed;
    tmp = (rcv->begin + rcv->data_size) % rcv->buffer_size;
    memcpy(&rcv->buffer1[tmp], &rcv->buffer2[0],
            min(readed, rcv->buffer_size - tmp));
    if (min(readed, rcv->buffer_size - tmp) != readed)
        memcpy( &rcv->buffer1[0], 
                &rcv->buffer2[rcv->buffer_size - tmp],
                readed - (rcv->buffer_size - tmp));
    rcv->data_size += readed;
    return readed;
}


int
my_reciever_write(My_reciever *rcv, size_t size)
{
    int tmp = min(size, rcv->data_size);
    if (tmp == 0)
        return 0;
    memcpy(&rcv->buffer2[0], &rcv->buffer1[rcv->begin],
            min(tmp, rcv->buffer_size - rcv->begin));
    if (min(tmp, rcv->buffer_size - rcv->begin) != tmp)
        memcpy(&rcv->buffer2[rcv->buffer_size - rcv->begin],
                &rcv->buffer1[0],
                tmp - (rcv->buffer_size - rcv->begin));
    int wrote = send(rcv->reciever_fd, rcv->buffer2, tmp, MSG_NOSIGNAL);
    if (wrote <= 0)
        return wrote;
    rcv->begin = (rcv->begin + wrote) % rcv->buffer_size;
    rcv->data_size -= wrote;
    return wrote;
}


void
my_reciever_print(My_reciever *rcv)
{
    printf("# Print:\n");
    printf("\t# begin:%d\tdata_size:%ld\tbuffer_size:%ld\n",
            rcv->begin, rcv->data_size, rcv->buffer_size);
    printf("\t# sender_fd:%d\treciever_fd:%d\n",
            rcv->sender_fd, rcv->reciever_fd);
    printf("\t# buffer1:");
    for (size_t i = 0; i < rcv->buffer_size; ++i)
        printf("%c", rcv->buffer1[i]);
    printf("\t# buffer2:");
    for (size_t i = 0; i < rcv->buffer_size; ++i)
        printf("%c", rcv->buffer2[i]);
    printf("\n\n");
    fflush(stdout);
}

int
my_reciever_is_full(My_reciever *rcv)
{
    return rcv->buffer_size == rcv->data_size;
}

int
my_reciever_is_empty(My_reciever *rcv)
{
    return rcv->data_size == 0;
}

void
my_reciever_close(My_reciever *rcv)
{
    rcv->closed = 1;
}

int
my_reciever_is_closed(My_reciever *rcv)
{
    return rcv->closed;
}

int
my_reciever_get_reciever(My_reciever *rcv)
{
    return rcv->reciever_fd;
}
