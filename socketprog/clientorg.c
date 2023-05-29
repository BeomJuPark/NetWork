#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
	struct hostent *hostp;
	struct sockaddr_in server;
	int sock;
	static struct timeval timeout = { 5, 0 }; /* five seconds */
	fd_set rmask, xmask, mask;
	char buf[BUFSIZ];
	int nfound, bytesread;
	if (argc != 3) {
		(void) fprintf(stderr,"usage: %s host port\n",argv[0]);
		exit(1);
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}

	if ((hostp = gethostbyname(argv[1])) == 0) {
		fprintf(stderr,"%s: unknown host\n",argv[1]);
		exit(1);
	}
	memset((void *) &server, 0, sizeof server);
	server.sin_family = AF_INET;
	memcpy((void *) &server.sin_addr, hostp->h_addr, hostp->h_length);
	server.sin_port = htons((u_short)atoi(argv[2]));
	if (connect(sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
		(void) close(sock);
		perror("connect");
		exit(1);
	}

	FD_ZERO(&mask);
	FD_SET(sock, &mask);
	FD_SET(fileno(stdin), &mask);
	for (;;) {
		rmask = mask;
		nfound = select(FD_SETSIZE, &rmask, (fd_set *)0, (fd_set *)0, &timeout);
		if (nfound < 0) {
			/* something is very wrong! */
			perror("select");
			exit(1);
		}
		if (FD_ISSET(fileno(stdin), &rmask)) {
			/* data from keyboard */
			if (!fgets(buf, sizeof (buf) - 1, stdin)) {
				if (ferror(stdin)) {
					perror("stdin");
					exit(1);
				}
				exit(0);
			}
			if (write(sock, buf, strlen(buf)) < 0) {
				perror("write");
				exit(1);
			}
		}
		if (FD_ISSET(sock, &rmask)) {
			/* data from network */
			bytesread = read(sock, buf, sizeof (buf) - 1);
			if ( bytesread == 0) {
				perror("close connection");
				exit(1);
			}
			buf[bytesread] = '\0';
			printf("%s: got %d bytes: %s\n", argv[0], bytesread, buf);
		}
	}
} /* main - clientorg.c */

