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


void forward_data(int from_descriptor, int to_descriptor, sockaddr_in from_address, sockaddr_in to_address, socklen_t addresses_len){
    fd_set readset;
    int result;
    char buf[BUFSIZE];
    
    printf("Forwarding fd %d to %d\n", from_descriptor, to_descriptor);
    
    for (;;){
        /* Call select() */
        do {
            FD_ZERO(&readset);
            FD_SET(from_descriptor, &readset);
            result = select(from_descriptor + 1, &readset, NULL, NULL, NULL);
        } while (result == -1 && errno == EINTR);
        
        if (result > 0) {
            if (FD_ISSET(from_descriptor, &readset)) {
                /* The socket_fd has data available to be read */
                result = (int)recv(from_descriptor, buf, BUFSIZE, 0);
                if (result == 0) {
                    /* This means the other side closed the socket */
                    printf("Empty result");
                }
                else {
                    buf[result] = 0;
                    printf("Resut: %s\n", buf);
                    if (sendto(to_descriptor, buf, strlen(buf), 0, (struct sockaddr *)&to_address, addresses_len) < 0){
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
    struct sockaddr_in socket1, socket2;
    socklen_t addresses_len = sizeof(socket1);
    int socket_descriptor1, socket_descriptor2;

    /* SOCKET 1 */
    /* Create socket */
    if ((socket_descriptor1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR: Cannot create socket\n");
        return 0;
    }
    memset((char *)&socket1, 0, sizeof(socket1));
    socket1.sin_family = AF_INET;
    socket1.sin_addr.s_addr = htonl(INADDR_ANY);
    socket1.sin_port = htons(0);
    
    /* Bind socket */
    if (bind(socket_descriptor1, (struct sockaddr *)&socket1, sizeof(socket1)) < 0) {
        perror("ERROR: Bind failed\n");
        return 0;
    }
    
    
    /* SOCKET 2 */
    /* Create socket */
    if ((socket_descriptor2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR: Cannot create socket\n");
        return 0;
    }
    memset((char *)&socket2, 0, sizeof(socket2));
    socket2.sin_family = AF_INET;
    socket2.sin_addr.s_addr = htonl(INADDR_ANY);
    socket2.sin_port = htons(PORT2);
    
    /* Bind socket */
    if (bind(socket_descriptor2, (struct sockaddr *)&socket2, sizeof(socket2)) < 0) {
        perror("ERROR: Bind failed\n");
        return 0;
    }
    
    /* Forward one way */
    std::thread t1(forward_data, socket_descriptor2, socket_descriptor1, socket1, socket2, addresses_len);

    /* Forward the other way */
    forward_data(socket_descriptor1, socket_descriptor2, socket2, socket1, addresses_len);
    
    
}
