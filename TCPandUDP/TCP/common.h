#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <poll.h>


#define SERVER_PORT 7766
#define BUF_MAX_LEN 4096
#define LISTEN_QUEUE_LEN 128

void error(int status, int err, char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    
    if (err)
        fprintf(stderr, ": %s (%d)\n", strerror(err), err);
        
    if (status)
        exit(status);
}

void setnoblocking(int fd) {
    int opts = 0;
    opts = fcntl(fd, F_GETFL);
    opts = opts | O_NONBLOCK;
    fcntl(fd, F_SETFL, opts);
}


int tcp_client(char *address, int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &server_addr.sin_addr);

    socklen_t len_server_addr = sizeof(server_addr);
    int ret = connect(socket_fd, (struct sockaddr *)&server_addr, len_server_addr);
    if (ret < 0)
        error(1, errno, "connect failed!");

    return socket_fd;
}

int tcp_server_listen(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        error(1, errno, "create socket failed!");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
        error(1, errno, "bind failed ");

    ret = listen(listen_fd, LISTEN_QUEUE_LEN);
    if (ret < 0) {
        error(1, errno, "listen failed ");
    }

    return listen_fd;
}


#endif