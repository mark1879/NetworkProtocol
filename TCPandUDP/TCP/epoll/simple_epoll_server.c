#include "common.h"
// #include <sys/epoll.h>

#define SERVER_PORT         (7766)
#define EPOLL_MAX_NUM       (2048)
#define BUFFER_MAX_LEN      (4096)

void str_toupper(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i ++) {
        str[i] = toupper(str[i]);
    }
}

int main() {

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        printf("Error, create socket failed: %d\n", listen_fd);
        return 1;
    }

    setnoblocking(listen_fd);

    int ret = 0;
    int epfd = 0; 

    do {

        int on = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (ret < 0) {
            print_error_msg("Error, create socket failed, listen_fd:", listen_fd);
            break;
        }

        ret = listen(listen_fd, 10);
        if (ret < 0) {
            print_error_msg("Error, listen failed, ret:", ret);
            break;
        }

        epfd = epoll_create(EPOLL_MAX_NUM);
        if (epfd < 0) {
            print_error_msg("Error, epoll create failed, epfd:", epfd);
            break;
        }

        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = listen_fd;

        ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &event);
        if ( ret < 0) {
            print_error_msg("Error: epoll_ctl add listen_fd failed, ret:", ret);
            break;
        }

        struct epoll_event* list_events = (epoll_event*)calloc(EPOLL_MAX_NUM, sizeof(epoll_event)); 

        struct sockaddr_in client_addr;
        char buffer[BUFFER_MAX_LEN];
       
        while (1) {
            int active_fds_cnt = epoll_wait(epfd, list_events, EPOLL_MAX_NUM, -1);
            printf("epoll_wait wakeup!\n");

            for (int i = 0; i < active_fds_cnt; i++) {
                if ((list_events[i].events & EPOLLERR) ||
                        (list_events[i].events & EPOLLHUP) ||
                        (!(list_events[i].events & EPOLLIN))){
                    print_error_msg("epoll error, fd: ", list_events[i].data.fd);
                    close(list_events[i].data.fd);
                    continue;

                } else if (list_events[i].data.fd == listen_fd) {
                    socklen_t   client_len;
                    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        print_error_msg("Error, accept failed, client_fd:", client_fd);
                        continue;
                    }

                    char ip[20];
                    printf("new connection[%s:%d]\n", inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)), ntohs(client_addr.sin_port));

                    setnoblocking(client_fd);

                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = client_fd;
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event);
                    if ( ret < 0) {
                        print_error_msg("Error: epoll_ctl add listen_fd failed, ret:", ret);
                    }

                } else if (list_events[i].events & EPOLLIN) {

                    int client_fd = list_events[i].data.fd;
                    memset(buffer, 0, BUFFER_MAX_LEN);

                    while (1) {
                        int read_bytes = read(client_fd, buffer, BUFFER_MAX_LEN - 1);
                        printf("read, read_bytes: %d\n", read_bytes);
                        if (read_bytes < 0) {
                            if (errno != EAGAIN) {
                                print_error_msg("Error, read failed, client_fd: ", client_fd);
                                close(client_fd);
                                epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &list_events[i]);
                            }
                            break;
                        } else if (read_bytes == 0) {
                            print_error_msg("Error, read failed, client_fd: ", client_fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &list_events[i]);
                            close(client_fd);
                            break;
                        } else {
                            printf("[read]: %s\n", buffer);
                            buffer[read_bytes] = '\0';
         
                            str_toupper(buffer);
                            int write_bytes = write(client_fd, buffer, strlen(buffer));
                            if (write_bytes < 0) {
                                print_error_msg("Error, write failed, client_fd: ", client_fd);
                            }
                        }
                    }
                }
            }
        }

    } while(0);

    close(epfd);
    close(listen_fd);

    return ret;
}