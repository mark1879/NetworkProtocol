#include "common.h"

int main() {

    int listen_fd = tcp_server_listen(SERVER_PORT, YES);
 
    int epoll_fd = epoll_create(EPOLL_MAX_NUM);
    if (epoll_fd < 0)
        error(1, errno, "epoll create failed!");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd;

    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);
    if (ret < 0) 
        error(1, errno, "epoll_ctl add listen_fd failed!");

    struct epoll_event* list_events = (struct epoll_event*)calloc(EPOLL_MAX_NUM, sizeof(event)); 

    struct sockaddr_in client_addr;
    char buffer[BUF_MAX_LEN];
    
    while (1) {
        int active_fds_cnt = epoll_wait(epoll_fd, list_events, EPOLL_MAX_NUM, -1);
        printf("epoll_wait wakeup!\n");

        for (int i = 0; i < active_fds_cnt; i++) {
            if ((list_events[i].events & EPOLLERR) ||
                    (list_events[i].events & EPOLLHUP) ||
                    (!(list_events[i].events & EPOLLIN))){
                close(list_events[i].data.fd);
                error(0, errno, "epoll error!");

            } else if (list_events[i].data.fd == listen_fd) {
                socklen_t   client_len;
                int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd < 0)
                    error(1, errno, "accept failed!");

                char ip[20];
                printf("new connection[%s:%d]\n", inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)), ntohs(client_addr.sin_port));
                printf("client_fd: %d\n", client_fd);

                setnoblocking(client_fd);

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                if (ret < 0)
                    error(1, errno, "epoll_ctl add client_fd failed!");

            } else if (list_events[i].events & EPOLLIN) {

                int client_fd = list_events[i].data.fd;
                printf("Got event on client_fd: %d\n", client_fd);
                memset(buffer, 0, BUF_MAX_LEN);

                while (1) {
                    int read_bytes = read(client_fd, buffer, BUF_MAX_LEN - 1);
                    printf("read, read_bytes: %d\n", read_bytes);

                    if (read_bytes < 0) {
                        error(0, errno, "read failed!");
                        break;
                    } else if (read_bytes == 0) {
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, &list_events[i]);
                        close(client_fd);
                        error(0, errno, "client has been terminated!");
                        break;
                    } else {
                        printf("[read]: %s\n", buffer);
                        buffer[read_bytes] = '\0';
        
                        str_toupper(buffer);
                        int write_bytes = write(client_fd, buffer, strlen(buffer));
                        printf("write bytes: %d\n", write_bytes);
                        if (write_bytes < 0)
                            error(0, errno, "write failed!");
                        else if (write_bytes == 0)
                            error(0, errno, "client has been terminated!");
                    }
                }
            }
        }
    }


    close(epoll_fd);
    close(listen_fd);
    free(list_events);

    return ret;
}