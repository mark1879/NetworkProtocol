//
//  main.cpp
//  Server
//
//  Created by MarkWu on 2022/6/24.
//

#include "common.h"

int main(int argc, char **argv) {

    int listen_fd = tcp_server_listen(SERVER_PORT, NO);

    struct sockaddr_in client_addr;
    socklen_t   client_len;

    printf("listening...\n");

    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
        error(1, errno, "accept failed!");

    char recv_buf[BUF_MAX_LEN];

    int run = 1;

    while (run) 
    {
        int recv_bytes = (int)read(client_fd, recv_buf, BUF_MAX_LEN - 1);
        printf("recv bytes: %d\n", recv_bytes);

        if (recv_bytes > 0) 
        {
            recv_buf[recv_bytes] = 0;
            printf("recv: %s\n", recv_buf);

            str_toupper(recv_buf);

            int write_bytes = write(client_fd, recv_buf, strlen(recv_buf));
            printf("write bytes: %d\n", write_bytes);
        
            if (write_bytes < 0)
                error(0, errno, "write failed!");
            else if (write_bytes == 0)
            {
                run = 0;
                error(0, errno, "client has been terminated!");
            }
        } 
        else if (recv_bytes == 0) 
        {
            run = 0;
            error(0, errno, "client has been terminated!");
        } 
        else
        {
            error(0, errno, "recv failed!");
        }
    }
   
    close(client_fd);
    close(listen_fd);

    printf("close socket!\n");

    return 0;
}