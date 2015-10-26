//
//  client.cpp
//  chat
//
//  Created by Ramiro Meares on 10/25/15.
//  Copyright © 2015 Ramiro Meares. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "port.h"

#define BUFLEN 2048
#define MSGS 5

int main(void)
{
    struct sockaddr_in my_address, remote_address;
    int fd, i, slen=sizeof(remote_address);
    socklen_t addresses_len = sizeof(remote_address);
    char buf[BUFLEN];
    int recvlen;
    char *server = "127.0.0.1";
    
    /* create a socket */
    
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("socket created\n");
    
    /* bind it to all local addresses and pick any port number */
    
    memset((char *)&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
        perror("bind failed");
        return 0;
    }
    
    /* now define remaddr, the address to whom we want to send messages */
    /* For convenience, the host address is expressed as a numeric IP address */
    /* that we will convert to a binary format via inet_aton */
    
    memset((char *) &remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(SERVICE_PORT);
    if (inet_aton(server, &remote_address.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
    /* now let's send the messages */
    
    for (i=0; i < MSGS; i++) {
        printf("Sending packet %d to %s port %d\n", i, server, SERVICE_PORT);
        sprintf(buf, "This is packet %d", i);
        if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remote_address, addresses_len)==-1) {
            perror("sendto");
            exit(1);
        }
        /* now receive an acknowledgement from the server */
        recvlen = (int)recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remote_address, &addresses_len);
        if (recvlen >= 0) {
            buf[recvlen] = 0;	/* expect a printable string - terminate it */
            printf("received message: \"%s\"\n", buf);
        }
    }
    close(fd);
    return 0;
}
