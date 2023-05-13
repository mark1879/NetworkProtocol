#include "common.h"

int main(int argc, char** argv) {
    if (argc != 2)
        error(1, 0, "usage: select <IPadress>\n");

    int socket_fd = tcp_client(argv[1], SERVER_PORT);

    setnoblocking(socket_fd);

    fd_set all_fds;

    FD_ZERO(&all_fds);
    FD_SET(0, &all_fds);
    FD_SET(socket_fd, &all_fds);

    char recv_buf[BUF_MAX_LEN];
    char send_buf[BUF_MAX_LEN];

    for(;;) {
        fd_set read_mask = all_fds;

        int num = select(socket_fd + 1, &read_mask, NULL, NULL, NULL);
        if (num < 0) 
            error(1, errno, "select failed!");

        if (FD_ISSET(socket_fd, &read_mask)) {
            ssize_t read_bytes = read(socket_fd, recv_buf, BUF_MAX_LEN - 1);

            if (read_bytes > 0) {
                recv_buf[read_bytes] = '\0';
                printf("recv: %s\n", recv_buf);
            }
            else if (read_bytes == 0)
                error(1, 0, "server has been terminated!");
            else 
                error(0, errno, "read failed!");
        }

        if (FD_ISSET(STDIN_FILENO, &read_mask)) {
            if (fgets(send_buf, BUF_MAX_LEN - 2, stdin) != NULL) {
                printf("sending: %s\n", send_buf);

                ssize_t write_bytes = write(socket_fd, send_buf, strlen(send_buf));
                printf("write bytes: %zd\n", write_bytes);
                if (write_bytes == 0) {
                    error(1, errno, "server has been terminated!");
                } else {
                    error(0, errno, "write failed!");
                }
            }
        }
    }

    return 0;
}