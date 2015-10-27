//
//  main.cpp
//  chat
//
//  Created by Ramiro Meares on 10/25/15.
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
#define SERVICE_PORT    21234


void recieve_messages(int socket_descriptor, sockaddr_in remote_address, socklen_t addresses_len){
    int recvlen;			/* # bytes received */
    char buf[BUFSIZE];      /* receive buffer */
    for (;;) {
        recvlen = (int)recvfrom(socket_descriptor, buf, BUFSIZE, 0, (struct sockaddr *)&remote_address, &addresses_len);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("Peer: %s\n", buf);
        }
        else
            printf("ERROR: Recieved invalid message\n");
    }
}


int main(int argc, const char * argv[]) {
    struct sockaddr_in my_address, remote_address;
    socklen_t addresses_len = sizeof(remote_address);
    int socket_descriptor;
    int service_port;
    char *server;
    bool is_server;
    
    if (argc < 1) {
        service_port = SERVICE_PORT;
        is_server = true;
    } else if (argc < 2) {
        service_port = atoi(argv[0]);
        is_server = true;
    } else {
        service_port = atoi(argv[0]);
        strcpy(server, argv[1]);
        is_server = false;
    }
    
    if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR: Cannot create socket\n");
        return 0;
    }
    
    memset((char *)&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (is_server){
        my_address.sin_port = htons(service_port);
    }else{
        my_address.sin_port = htons(0);
    }


    if (bind(socket_descriptor, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
        perror("ERROR: Bind failed\n");
        return 0;
    }
    
    if (!is_server){
        memset((char *) &remote_address, 0, sizeof(remote_address));
        remote_address.sin_family = AF_INET;
        remote_address.sin_port = htons(service_port);
        if (inet_aton(server, &remote_address.sin_addr)==0) {
            perror("ERROR: inet_aton() failed\n");
            return 0;
        }
        printf("Conntected to server %s in port %d\n", server, service_port);
    }else{
        printf("Created server in port %d\n", service_port);        
    }
    
    std::thread t1(recieve_messages, socket_descriptor, remote_address, addresses_len);
    
    char buf[BUFSIZE];      /* send buffer */
    for (;;) {
        std::cin >> buf;
        printf("Me: %s\n", buf);
        if (sendto(socket_descriptor, buf, strlen(buf), 0, (struct sockaddr *)&remote_address, addresses_len) < 0)
            perror("ERROR: sendto() failed\n");
        
    }
    
    return 0;
}

