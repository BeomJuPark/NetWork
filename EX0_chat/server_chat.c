#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_CLIENTS 10 // MAx Client cnt

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
        int client_sockets[MAX_CLIENTS]; //Client socket array
        int client_cnt = 0; //Current Client cnt
	int i, nfound, fd, maxfd, bytesread, addrlen;
	fd_set rmask, mask;
	static struct timeval timeout = { 5, 0 }; /* 5 seconds */

	char buf[BUFSIZ];
	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s port\n",argv[0]);
		exit(1);
	}
	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}

	memset((void *) &server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons((u_short)atoi(argv[1]));
	if (bind(request_sock, (struct sockaddr *)&server, sizeof server) < 0) {
		perror("bind");
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}

	FD_ZERO(&mask);
	FD_SET(request_sock, &mask);
	maxfd = request_sock;
	for (;;) {
		rmask = mask;
		nfound = select(maxfd + 1, &rmask, (fd_set *)0, (fd_set *)0, &timeout);
		if (nfound < 0) {
			if (errno == EINTR) {
				printf("interrupted system call\n");
				continue;
			}
			/* something is very wrong! */
			perror("select");
			exit(1);
		}
		if (FD_ISSET(request_sock, &rmask)) {
			/* a new connection is available on the connetion socket */
			addrlen = sizeof(remote);
			new_sock = accept(request_sock,
				(struct sockaddr *)&remote, (socklen_t *)&addrlen);
			if (new_sock < 0) {
				perror("accept");
				exit(1);
			}
			printf("connection from host %s, port %d, socket %d\n",
				inet_ntoa(remote.sin_addr), ntohs(remote.sin_port),
				new_sock);
                        char info_msg[BUFSIZ];
                        snprintf(info_msg, sizeof(info_msg), "Server IP: %s, Port : %d \n",inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
                        write(new_sock, info_msg, strlen(info_msg));
                        client_sockets[client_cnt++] = new_sock; //new client add
			FD_SET(new_sock, &mask);
			if (new_sock > maxfd)
				maxfd = new_sock;
			FD_CLR(request_sock, &rmask);
		}
		for (fd = 4; fd <= maxfd ; fd++) {
			/* look for other sockets that have data available */
			if (FD_ISSET(fd, &rmask)) {
				/* process the data */
				bytesread = read(fd, buf, sizeof (buf) - 1);
				if (bytesread < 0) {
					perror("read");
					/* fall through */
				}
				if (bytesread<=0) {
			          printf("server: end of file on %d\n",fd);
				  FD_CLR(fd, &mask);
				  if (close(fd)) perror("close");
                                  for(i = 0 ; i < client_cnt ; i++)
                                  {
                                    if(client_sockets[i] == fd)
                                    {
                                        client_sockets[i] = client_sockets[client_cnt-1];
                                        break;
                                    }
                                  }

                                  client_cnt--;
                                  break;
			        }
	                        buf[bytesread] = '\0';

                                for( i = 0 ; i < client_cnt; i++)
                                {
                                   if(client_sockets[i] != fd)
                                   {
                                     if(write(client_sockets[i], buf, bytesread) != bytesread)
                                       perror("echo");
                                   }
                                }
                        }
                }
        }
}



