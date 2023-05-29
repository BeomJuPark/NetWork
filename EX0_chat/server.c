#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define MAX_CLIENTS 10 // MAx Client cnt

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
        int client_sockets[MAX_CLIENTS]; //Client socket array
        int client_cnt = 0; //Current Client cnt
	int bytesread, addrlen;
	int i;
	char buf[BUFSIZ];
	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s port\n", argv[0]);
		exit(1);
	}
	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}

	memset((void *) &server, 0, sizeof (server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	printf("%d\n", INADDR_ANY);
	server.sin_port = htons((u_short)atoi(argv[1]));

	if (bind(request_sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
		perror("bind");
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}
	for (;;) {
		addrlen = sizeof(remote);
		new_sock = accept(request_sock,	(struct sockaddr *)&remote, (socklen_t *)&addrlen);
		if (new_sock < 0) {
			perror("accept");
			exit(1);
		}
		printf("connection from host %s, port %d, socket %d\n",	inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);
                char info_msg[BUFSIZ];
                snprintf(info_msg, sizeof(info_msg), "Server IP: %s, Port : %d \n",
                    inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
                write(new_sock, info_msg, strlen(info_msg));
                
                client_sockets[client_cnt++] = new_sock; //new client add

		for (;;) {
			bytesread = read(new_sock, buf, sizeof (buf) - 1);
			if (bytesread<=0) {
				printf("server: end of file on %d\n", new_sock);
				if (close(new_sock)) 
					perror("close");

                                //If Client disable , disconnect
                                for(i = 0 ; i < client_cnt ; i++)
                                {
                                  if(client_sockets[i] == new_sock)
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
                           if(client_sockets[i] != new_sock)
                           {
                             if(write(client_sockets[i], buf, bytesread) != bytesread)
                               perror("echo");
                           }
                        }
                }
        }
}


