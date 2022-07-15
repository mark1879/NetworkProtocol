#include "common.h"

int main(int argc, char** argv) {
    if (argc != 2)
        error(1, 0, "usage: select <IPadress>\n");

    int socket_fd = tcp_client(argv[1], SERVER_PORT);

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

            if (read_bytes < 0)
                error(1, errno, "read error!");
            else if (read_bytes == 0)
                error(1, 0, "server terminated!");

            recv_buf[read_bytes] = '\0';
            fputs(recv_buf, stdout);
            fputs("\n", stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &read_mask)) {
            if (fgets(send_buf, BUF_MAX_LEN - 2, stdin) != NULL) {
                printf("now sending: %s\n", send_buf);

                ssize_t write_bytes = write(socket_fd, send_buf, strlen(send_buf));
                if (write_bytes < 0)
                    error(1, errno, "write failed!");

                printf("sended bytes: %zd\n", write_bytes);
            }
        }
    }

    return 0;
}