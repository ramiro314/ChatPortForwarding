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
#define SERVICE_PORT    21200


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
    char *server = nullptr;
    bool is_server;
    char buf[BUFSIZE];      /* send buffer */
    
    int i;
    for (i = 0; i < argc; i++)
        printf("Param %d: %s\n", i, argv[i]);
    
    /* Set default values and identify if we are server */
    if (argc < 2) {
        service_port = SERVICE_PORT;
        is_server = true;
    } else if (argc < 3) {
        service_port = atoi(argv[1]);
        is_server = true;
    } else {
        service_port = atoi(argv[1]);
        server = (char*)argv[2];
        is_server = false;
    }
    
    /* Create socket */
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

    /* Bind socket */
    if (bind(socket_descriptor, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
        perror("ERROR: Bind failed\n");
        return 0;
    }
    
    /* If we are client, connect to server */
    if (!is_server){
        memset((char *) &remote_address, 0, sizeof(remote_address));
        remote_address.sin_family = AF_INET;
        remote_address.sin_port = htons(service_port);
        if (inet_aton(server, &remote_address.sin_addr)==0) {
            perror("ERROR: inet_aton() failed\n");
            return 0;
        }
        /* Make first call to the server */
        if (sendto(socket_descriptor, buf, strlen(buf), 0, (struct sockaddr *)&remote_address, addresses_len) < 0)
            perror("ERROR: sendto() failed\n");
        printf("Conntected to server %s in port %d\n", server, service_port);
    }else{
        printf("Created server in port %d\nWaiting for connection\n", service_port);
        recvfrom(socket_descriptor, buf, BUFSIZE, 0, (struct sockaddr *)&remote_address, &addresses_len);
        printf("Connected with client\n");
    }
    
    /* Initiate message reciever thread */
    std::thread t1(recieve_messages, socket_descriptor, remote_address, addresses_len);
    
    for (;;) {
        std::cin >> buf;
        if (sendto(socket_descriptor, buf, strlen(buf), 0, (struct sockaddr *)&remote_address, addresses_len) < 0)
            perror("ERROR: sendto() failed\n");
        
    }
    
    return 0;
}

