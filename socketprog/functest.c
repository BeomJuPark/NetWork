#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#include <netinet/tcp.h>
#include <errno.h> 


// getsockopt test
int getsockopttest(int sockfd) {
    int optnames[128] = {
	    SO_ACCEPTCONN, SO_BROADCAST, SO_DEBUG, SO_ERROR, SO_KEEPALIVE,
	    SO_LINGER, SO_OOBINLINE, SO_RCVBUF, SO_REUSEADDR, SO_REUSEPORT, 
	    SO_SNDBUF, SO_TYPE, SO_PROTOCOL, TCP_NODELAY, SO_DOMAIN,
            SO_DONTROUTE, -1
    };

    int optval = 0;
    int i = 0;
    socklen_t len = sizeof(optval);

    printf("Socket : %d\n", sockfd);
    for (i = 0; i < 100; i++) {
        if(optnames[i] == -1) 
            break;
        optval = 0;
        len = sizeof(optval);
	//if (getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &len) == -1) {
        if (getsockopt(sockfd, SOL_SOCKET, optnames[i], &optval, &len) == -1) {
            printf("%d: error %d\n", i, errno);
        } else {
            printf("%d: %d\n", i, optval);
	}
    }

    return 0;
}

// setsockopt test
int setsockopttest(int sockfd) {

	int keepalive = 1;
	socklen_t len = sizeof(keepalive);
	if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, len) == -1) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	int reuse = 1;
	len = sizeof(reuse);
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, len) == -1) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, len) == -1) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	return 0;
}

// address related functions
int addresstest(struct sockaddr_in *addr) {
    int val = 0;
    in_addr_t t;
    char *ipaddr = "10.4.2.1";
    char ipaddr2[100];
    struct in_addr inp;

    printf("host %s, port %d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

    val = inet_aton(ipaddr, &inp);
    printf("%08x %d\n", ntohl(inp.s_addr), val);

    t = inet_addr(ipaddr);
    printf("%08X %s\n", t, ipaddr);
    t = inet_network(ipaddr);
    printf("%08X %s\n", t, ipaddr);

    printf("inet_ntoa %s\n", inet_ntoa(inp));

    printf("inet_lnaof %08X\n", inet_lnaof(inp));
    printf("inet_netof %08X\n", inet_netof(inp));

    inp = inet_makeaddr(10, 4 << 16 | 2 << 8 | 2);
    printf("inet_makeaddr %s\n", inet_ntoa(inp));

    printf("inet_lnaof %08X\n", inet_lnaof(inp));
    printf("inet_netof %08X\n", inet_netof(inp));

    memset(&inp, 0, sizeof(inp));
    inet_pton(AF_INET, ipaddr, &inp);
    printf("inet_pton %s %08X \n", ipaddr, inp.s_addr);

}

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
	int bytesread, addrlen;
	int i, ret;
	char buf[BUFSIZ];
	if (argc != 3) {
		(void) fprintf(stderr,"usage: %s IP port\n", argv[0]);
		exit(1);
	}
	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}

	memset((void *) &server, 0, sizeof (server));
	server.sin_family = AF_INET;
	//server.sin_addr.s_addr = INADDR_ANY;
        inet_aton(argv[1], &server.sin_addr);
	server.sin_port = htons((u_short)atoi(argv[2]));

        setsockopttest(request_sock);
	if (bind(request_sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
		perror("bind");
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}
	addresstest(&server);
        getsockopttest(request_sock);
	for (;;) {
		addrlen = sizeof(remote);
		new_sock = accept(request_sock, (struct sockaddr *)&remote, (socklen_t *)&addrlen);
		if (new_sock < 0) {
			exit(1);
		}
		printf("connection from host %s, port %d, socket %d\n",
			inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);
                getsockopttest(request_sock);
                getsockopttest(new_sock);

	}
}


