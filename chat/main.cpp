//
//  main.cpp
//  chat
//
//  Created by Ramiro Meares on 10/25/15.
//  Copyright © 2015 Ramiro Meares. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "port.h"

#define BUFSIZE 2048

int main(int argc, const char * argv[]) {
    struct sockaddr_in my_addres;
    struct sockaddr_in remote_address;
    socklen_t addresses_len = sizeof(remote_address);
    int recvlen;			/* # bytes received */
    int msg_cnt = 0;
    char buf[BUFSIZE];	/* receive buffer */
    int fd; /* socket */
    
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return 0;
    }
    
    memset((char *)&my_addres, 0, sizeof(my_addres));
    my_addres.sin_family = AF_INET;
    my_addres.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addres.sin_port = htons(SERVICE_PORT);
    
    if (bind(fd, (struct sockaddr *)&my_addres, sizeof(my_addres)) < 0) {
        perror("bind failed\n");
        return 0;
    }
    
    for (;;) {
        printf("waiting on port %d\n", SERVICE_PORT);
        recvlen = (int)recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remote_address, &addresses_len);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);
        }
        else
            printf("uh oh - something went wrong!\n");
        sprintf(buf, "ack %d", msg_cnt++);
        printf("sending response \"%s\"\n", buf);
        if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remote_address, addresses_len) < 0)
            perror("sendto");
    }
    
    return 0;
}
