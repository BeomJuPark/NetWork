#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int main() {
    int i = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int optnames[100] = {SO_ACCEPTCONN, SO_BROADCAST, SO_DEBUG, SO_ERROR, SO_KEEPALIVE,
	    SO_LINGER, SO_OOBINLINE, SO_RCVBUF, SO_REUSEADDR, SO_SNDBUF, SO_TYPE, SO_KEEPALIVE, TCP_NODELAY, -1};

    int optval = 0;
    socklen_t len = sizeof(optval);

    for (i = 0; i < 100; i++) {
        if(optnames[i] == -1) 
            break;
        optval = 0;
        len = sizeof(optval);
	//if (getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &len) == -1) {
        if (getsockopt(sockfd, SOL_SOCKET, optnames[i], &optval, &len) == -1) {
            printf("error %d %d\n", i, errno);
        }

        printf("%d: %d\n", i, optval);
    }

    close(sockfd);
    return 0;
}
