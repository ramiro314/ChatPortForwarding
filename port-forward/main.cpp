//
//  main.cpp
//  port-forward
//
//  Created by Ramiro Meares on 11/15/15.
//  Copyright © 2015 Ramiro Meares. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

#define BUFSIZE 2048
#define PORT1 21200
#define PORT2 21201
#define INTERNAL_PORT1 21220
#define INTERNAL_PORT2 21221

int inline max(int x, int y){
    return x > y ? x: y;
}

void forward_data(int from_port, int to_port){
    
    fd_set readset;
    int result;
    char buf[BUFSIZE];
    struct sockaddr_in socket1, socket2, read_address, write_address1, write_address2;
    socklen_t addresses_len = sizeof(socket1);
    int socket_descriptor1, socket_descriptor2;
    
    /* SOCKET 1 */
    /* Create socket */
    if ((socket_descriptor1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR: Cannot create socket\n");
        exit(-1);
    }
    memset((char *)&socket1, 0, sizeof(socket1));
    socket1.sin_family = AF_INET;
    socket1.sin_addr.s_addr = htonl(INADDR_ANY);
    socket1.sin_port = htons(0);
    
    /* Bind socket */
    if (bind(socket_descriptor1, (struct sockaddr *)&socket1, sizeof(socket1)) < 0) {
        perror("ERROR: Bind failed\n");
        exit(-1);
    }
    
    /* SOCKET 2 */
    /* Create socket */
    if ((socket_descriptor2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR: Cannot create socket\n");
        exit(-1);
    }
    memset((char *)&socket2, 0, sizeof(socket2));
    socket2.sin_family = AF_INET;
    socket2.sin_addr.s_addr = htonl(INADDR_ANY);
    socket2.sin_port = htons(to_port);
    
    /* Bind socket */
    if (bind(socket_descriptor2, (struct sockaddr *)&socket2, sizeof(socket2)) < 0) {
        perror("ERROR: Bind failed\n");
        exit(-1);
    }
    
    /* Remote server sockets */
    memset((char *) &write_address1, 0, sizeof(write_address1));
    write_address1.sin_family = AF_INET;
    write_address1.sin_port = htons(0);
    if (inet_aton("127.0.0.1", &write_address1.sin_addr)==0) {
        perror("ERROR: inet_aton() failed\n");
        exit(-1);
    }

    memset((char *) &write_address2, 0, sizeof(write_address2));
    write_address2.sin_family = AF_INET;
    write_address2.sin_port = htons(from_port);
    if (inet_aton("127.0.0.1", &write_address2.sin_addr)==0) {
        perror("ERROR: inet_aton() failed\n");
        exit(-1);
    }
    
    printf("Forwarding fd %d to %d\n", from_port, to_port);
    
    for (;;){
        /* Call select() */
        do {
            FD_ZERO(&readset);
            FD_SET(socket_descriptor1, &readset);
            FD_SET(socket_descriptor2, &readset);
            result = select(max(socket_descriptor1, socket_descriptor2) + 1, &readset, NULL, NULL, NULL);
        } while (result == -1 && errno == EINTR);
        
        if (result > 0) {
            if (FD_ISSET(socket_descriptor1, &readset)) {
                /* Connects with server, behaves like client */
                printf("Reads from server, send to client\n");
                printf("Call comming from %s:%d going to %s:%d being redirected to %s:%d\n", inet_ntoa(read_address.sin_addr), ntohs(read_address.sin_port), inet_ntoa(socket1.sin_addr), ntohs(socket1.sin_port), inet_ntoa(write_address1.sin_addr), ntohs(write_address1.sin_port));
                result = (int)recvfrom(socket_descriptor1, buf, BUFSIZE, 0, (struct sockaddr *)&read_address, &addresses_len);
                if (result == 0) {
                    /* This means the other side closed the socket */
                    printf("Empty result\n");
                }
                else {
                    buf[result] = 0;
                    printf("Resut: %s\n", buf);
                    if (sendto(socket_descriptor2, buf, strlen(buf), 0, (struct sockaddr *)&write_address1, addresses_len) < 0){
                        perror("ERROR: sendto() failed\n");
                    }
                }
            }
            if (FD_ISSET(socket_descriptor2, &readset)) {
                /* Connects with client, behaves like server */
                printf("Reads from client, send to server\n");
                result = (int)recvfrom(socket_descriptor2, buf, BUFSIZE, 0, (struct sockaddr *)&read_address, &addresses_len);
                if (htons(0) == write_address1.sin_port){
                        write_address1.sin_port = read_address.sin_port;
                }
                printf("Call comming from %s:%d going to %s:%d being redirected to %s:%d\n", inet_ntoa(read_address.sin_addr), ntohs(read_address.sin_port), inet_ntoa(socket2.sin_addr), ntohs(socket2.sin_port), inet_ntoa(write_address2.sin_addr), ntohs(write_address2.sin_port));
                if (result == 0) {
                    /* This means the other side closed the socket */
                    printf("Empty result\n");
                }
                else {
                    buf[result] = 0;
                    printf("Resut: %s\n", buf);
                    if (sendto(socket_descriptor1, buf, strlen(buf), 0, (struct sockaddr *)&write_address2, addresses_len) < 0){
                        perror("ERROR: sendto() failed\n");
                    }
                }
            }

        }
        else if (result < 0) {
            /* An error ocurred, just print it to stdout */
            printf("Error on select(): %s", strerror(errno));
        }
    }
}


int main(int argc, const char * argv[]) {
    
    /* Check arguments */
    if(argc != 3) {
        printf("Usage: %s <FROM_PORT> <TO_PORT>\n", argv[0]);
        exit(-1);
    }
    
    forward_data(atoi(argv[1]), atoi(argv[2]));
    
}
