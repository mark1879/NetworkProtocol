#include "common.h"

#define MAX_POLL_SIZE 128

int main(int argc, char **argv) {

    int listen_fd = tcp_server_listen(SERVER_PORT);

    struct pollfd events_set[MAX_POLL_SIZE];
    events_set[0].fd = listen_fd;
    events_set[0].events = POLLRDNORM;

    for (int i = 1; i < MAX_POLL_SIZE; i++)
        events_set[i].fd = -1;      // -1 表示此位置还未被占用

    char recv_buf[BUF_MAX_LEN] = {0};

    for (;;) {
        int ready_num = poll(events_set, MAX_POLL_SIZE, -1);
        if (ready_num < 0)
            error(1, errno, "poll failed!");

        if (events_set[0].revents & POLLRDNORM) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) 
                error(1, errno, "accept failed!");

            int i = 1;
            for (; i < MAX_POLL_SIZE; i++) {
                if (events_set[i].fd < 0) {
                    events_set[i].fd = client_fd;
                    events_set[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i == MAX_POLL_SIZE)
                error(1, errno, "poll events_set overflows!");

            if (--ready_num <= 0)
                continue;
        }

        for (int i = 1; i < MAX_POLL_SIZE; i++) {
            int socket_fd = events_set[i].fd;

            if (socket_fd < 0)
                continue;

            if (events_set[i].revents & (POLLRDNORM | POLLERR)) {
                int recv_bytes = read(socket_fd, recv_buf,  BUF_MAX_LEN -1);
                if (recv_bytes > 0) {
                    recv_buf[recv_bytes] = 0;
                    printf("recv: %s\n\n", recv_buf);

                    str_toupper(recv_buf);

                    int write_bytes = write(socket_fd, recv_buf, strlen(recv_buf));
                    if (write_bytes <= 0) {
                        error(0, errno, "write failed!");
                    }
                } else if (recv_bytes == 0) {
                    close(socket_fd);
                    events_set[i].fd = -1;
                    error(0, errno, "client has been terminated!");
                } else {
                    error(0, errno, "read failed!");
                }

                if (--ready_num <= 0)
                    break;
            }
        }
    }

    return 0;
}