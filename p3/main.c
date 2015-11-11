#include <stdio.h>
#include "my_reciever.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
    int rfd = open("input", O_RDONLY);
    int wfd = open("output", O_WRONLY | O_CREAT | O_TRUNC, 0755);
    My_reciever *mrcv = my_reciever_init(10, wfd, rfd);
    my_reciever_print(mrcv);
    
    while(my_reciever_read(mrcv, 3) > 0)
    {
        my_reciever_print(mrcv);
        my_reciever_write(mrcv, 1);
        my_reciever_print(mrcv);
        if (my_reciever_is_full(mrcv))
        {
            printf("extra\n");
            my_reciever_write(mrcv, 10);
            my_reciever_print(mrcv);
        }
    }
    my_reciever_write(mrcv, 10);
    my_reciever_destroy(mrcv);
    close(rfd);
    close(wfd);
    return 0;
}
