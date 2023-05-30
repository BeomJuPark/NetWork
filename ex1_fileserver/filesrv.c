// Student ID :
// Name :

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


int portnum = 0;

void SendFile(int new_sock, char *fname) {
	char buf[BUFSIZ];
	char msg[1024];
	struct stat infobuf;              /* place to store info */

	/* get file info */
	if ( stat(fname, &infobuf) == -1 )  {
		/* file not found */
		printf("Server Error : No such file %s!\n", fname);
		sprintf(msg, "FILE NOT FOUND\r\n");
		infobuf.st_size = -1;
		if (write(new_sock, msg, strlen(msg))!=strlen(msg))
			perror("echo error");
		
	} else {
		if(infobuf.st_size != -1) {
			int s = 0;
			int num = 0;
			FILE *fp1 = fopen(fname, "r");	
			memset(buf, 0, sizeof(buf));
			while((num = fread( buf, sizeof( char ), BUFSIZ - 1, fp1 )) > 0) {
				s += num;
				if (write(new_sock, buf, num)!= num)
					perror("send file error");
			}
			fclose(fp1);
			printf("finish %d %d\n", s, (int)infobuf.st_size);
		}
	}
}


void SendDir(int new_sock, char *ext) {
	char buf[BUFSIZ];
	DIR *folder;
	struct dirent *entry;
	int num = 0;
	int fcnt = 0;

	folder = opendir(".");
	if(folder == NULL) {
		perror("Unable to read directory");
		return;
	}

	while( (entry = readdir(folder)) ) {
		char *p = strrchr(entry->d_name, '.');
		if(p == NULL)
			continue;
		if (strcmp(p + 1, ext) != 0) 
			continue;

		sprintf(buf, "%s\n", entry->d_name);
		num = strlen(buf);
		if (write(new_sock, buf, num) != num) {
			perror("send ls error");
		}
		fcnt++;
	}
	if (fcnt == 0) {
		sprintf(buf, "FILE NOT FOUND\r\n");
		num = strlen(buf);
		if (write(new_sock, buf, num) != num) {
			perror("send ls error");
		}
	}
	closedir(folder);
}


void StoreFile(FILE *fp, char *fname) {
	char buf[BUFSIZ];
	int num = 0;

	FILE *fp1 = fopen(fname, "w");	
	printf("fopen %s\n", fname);
	if (fp1 == NULL) {
		printf("fopen error for %s\n", fname);
		return;
	}
	while((num = fread( buf, sizeof( char ), BUFSIZ - 1, fp )) > 0) {
		buf[num] = 0;
		printf("%s\n", buf);
		if (fwrite(buf, sizeof(char), num, fp1) != num) {
			printf("write error %d\n", num);
			fclose(fp1);
			return;
		}
	}
	fclose(fp1);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
	socklen_t addrlen;
	char buf[BUFSIZ];

	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum \n",argv[0]);
		exit(1);
	}

	portnum = atoi(argv[1]);

	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket create error");
		exit(1);
	}
	
	printf("Student ID : 2000000\n");
	printf("Name : Sanghwan Lee\n");

	// Create a Server Socket
	memset((void *) &server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons((u_short)portnum);

	if (bind(request_sock, (struct sockaddr *)&server, sizeof server) < 0) {
		perror("bind error");
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen error");
		exit(1);
	}

	/* a new connection is available on the connetion socket */
	while(1) {
		addrlen = sizeof(remote);
		fflush(stdout); 
		new_sock = accept(request_sock,
			(struct sockaddr *)&remote, &addrlen);
		if (new_sock < 0) {
			perror("accept error");
			exit(1);
		}
		printf("Connection : Host IP %s, Port %d, socket %d\n",
			inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);


		FILE *fp = fdopen(new_sock, "r"); 
		buf[0] = 0;
		if (!fgets(buf, BUFSIZ - 1, fp)) {
			printf("No Data: close connection");
			fclose(fp);
			if (close(new_sock)) perror("close error");
			continue;
		}
		printf("%s", buf);
	
		// get the request from the client
		// parse the data from the client
		// give the file to the client
		fflush(stdout); 
		char *command = strtok(buf, " \t\n\r"); // Command
		char *filename = strtok(NULL, " \t\n\r");
					
		if (strcmp(command, "GET") == 0) {
			SendFile(new_sock, filename);
		} else if (strcmp(command, "LS") == 0) {
			SendDir(new_sock, filename);
		} else if (strcmp(command, "PUT") == 0) {
			StoreFile(fp, filename);
		} else {
			printf("Unknown Command %s\n", command);
		}
//		fclose(fp);

		if (close(new_sock)) 
			perror("close error2");
	}
} /* main - hw2sol.c */


