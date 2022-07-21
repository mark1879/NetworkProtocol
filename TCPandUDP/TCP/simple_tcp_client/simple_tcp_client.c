//
//  main.cpp
//  Client
//
//  Created by MarkWu on 2022/6/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"

int main(int argc, char **argv)
{
    char *server_ip = (char *)"192.168.2.16";
    if (argc >= 2) {
        server_ip = argv[1];
    }
    
    printf("server ip: %s\n", server_ip);
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 创建socket
    if (sockfd < 0)
        error(1, errno, "create socket failed!");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    int ret = inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    if (ret <= 0)
        error(1, errno, "inet_pton failed!");
    
    // // 绑定本地地址
    // struct sockaddr_in client_addr;
    // bzero(&client_addr, sizeof(client_addr));
    // client_addr.sin_family = AF_INET;
    // client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // client_addr.sin_port = htons(SERVER_PORT);
    
    // ret = bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr));
    // if (ret < 0)
    //     error(1, errno, "bind failed!");
    
    ret = connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if (ret < 0)
        error(1, errno, "connect failed!");

    setnoblocking(sockfd);

    char send_buf[BUF_MAX_LEN] = {0};
    char recv_buf[BUF_MAX_LEN] = {0};

    printf("Please input: \n");
    // fgets函数返回字符串，其中包含 '\n'，并且字符串末尾被添加 '\0'
    // 任何时候，最后一个字符都是 '\0'; 只有参数 size - 输入字符串长度 >= 2, 末尾才会添加 '\n'。
    while (fgets(send_buf, BUF_MAX_LEN - 2, stdin) != NULL)
    {
        printf("Send: %s strlen: %lu\n", send_buf, strlen(send_buf));
        
        if (strncmp(send_buf, "out", 3) == 0)
            break;
        
        // 把输入的字符串发送到服务器中去
        // strlen返回字符串的长度，但不包括\0
        int send_bytes = (int)send(sockfd, send_buf, strlen(send_buf), 0);
        if (send_bytes <= 0) 
            error(0, errno, "send failed!");
        
        sleep(2);
        
        while (1)
        {
            int recv_bytes = (int)read(sockfd, recv_buf, BUF_MAX_LEN  - 1);
            
            if (recv_bytes <= 0) {
                error(0, errno, "recv failed!");
                break;
            }else {
                printf("recv: %s\n", recv_buf);
            }
            
            memset(recv_buf, 0, BUF_MAX_LEN);
        }
        
        memset(send_buf, 0, BUF_MAX_LEN);
        
        printf("Please input: \n");
    }
    
    close(sockfd);

    return 0;
}