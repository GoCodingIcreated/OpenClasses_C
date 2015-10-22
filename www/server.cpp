#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <poll.h>
#include <event2/http.h>
#include <event2/util.h>
#include <event2/buffer.h>

char HELLO_MSG[] = "Welcome!\n";


void
get_call_back(struct evhttp_request *req, void *arg)
{
    char *s = (char*) calloc(1, strlen((char*)arg)
                        + strlen(evhttp_request_get_uri(req)));
    sprintf(s, "%s/%s", (char*)arg, evhttp_request_get_uri(req));
    struct evbuffer *buf = evbuffer_new();
    int fd = open(s, O_RDONLY);
    if (fd < 0)
    {
        free(s);
        evhttp_send_error(req, 404, NULL);
        return;
    }
    free(s);
    long long size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    evbuffer_add_file(buf, fd, 0, size);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

void
head_call_back(struct evhttp_request *req, void *arg)
{
    char *s = (char*) calloc(1, strlen((char*)arg)
                        + strlen(evhttp_request_get_uri(req)));
    sprintf(s, "%s/%s", (char*)arg, evhttp_request_get_uri(req));
    int fd = open(s, O_RDONLY);
    if (fd < 0)
    {
        evhttp_send_error(req, 404, NULL);
        free(s);
        return;
    }
    free(s);
    close(fd);
    evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}

void
post_call_back(struct evhttp_request *req, void *arg)
{
    char *s = (char *) calloc(1, strlen((char*)arg)
                        + strlen(evhttp_request_get_uri(req)));
    sprintf(s, "%s/%s", (char*)arg, evhttp_request_get_uri(req));
    int fd = open(s, O_WRONLY);
    if (fd > 0)
    {
        evhttp_send_error(req, 404, NULL);
        close(fd);
        free(s);
        return ;
    }
    fd = open(s, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd < 0)
    {
        free(s);
        evhttp_send_error(req, 404, NULL);
        return ;
    }
    free(s);
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    evbuffer_write(buf, fd);
    close(fd);
    evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}

void
request_call_back(struct evhttp_request * req, void *arg)
{
    evhttp_cmd_type type  = evhttp_request_get_command(req);
    const char *t = evhttp_request_get_uri(req);
    printf("%s\n", t);
    //struct evbuffer *buf = evhttp_request_get_output_buffer(req);
    if (type == EVHTTP_REQ_GET)
        get_call_back(req, arg);
    else if (type == EVHTTP_REQ_POST)
    {
        printf("OLOLO\n");
        post_call_back(req, arg);
    }
    else if (type == EVHTTP_REQ_HEAD)
        head_call_back(req, arg);
    else
        printf("OLOLO\n");
}

int
main(int argc, char **argv)
{
    char *ip, *dir_name;
    int port = 3010;
    
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            if (i == argc - 1 || 
                sscanf(argv[i++ + 1], "%d", &port) != 1)
            {
                printf("Invalid argument\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            if (i == argc - 1)
            {
                printf("Invalid argument\n");
                return 1;
            }
            ip = strdup(argv[i++ + 1]);
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            if (i == argc - 1)
            {
                printf("Invalid arguments\n");
                return 1;
            }
            dir_name = strdup(argv[i++ + 1]);
            if (chdir(dir_name) < 0)
            {
                printf("Invalid directory\n");
                return 1;
            }
        }
    }
    
    
    struct event_base* ev_base = event_base_new();
    struct evhttp *server = evhttp_new(ev_base);
    evhttp_bind_socket(server, ip, port);
    
    evhttp_set_gencb(server, request_call_back, dir_name);
    
    event_base_dispatch(ev_base);
    
    evhttp_free(server);
    event_base_free(ev_base);
    free(ip);
    free(dir_name);
    printf("loop ended.\n");
    return 0;
}
