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

#define BUFFER_SIZE 1024

void error(const char *message) {
    perror(message);
    exit(1);
}

void send_file(int client_socket, const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        error("Error opening file");
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            error("Error sending file");
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    fclose(file);
}

void receive_file(int client_socket, const char *file_name) {
    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        error("Error creating file");
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        if (fwrite(buffer, 1, bytes_received, file) < bytes_received) {
            error("Error writing to file");
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    fclose(file);
}
void list_files(int new_sock, char *ext) {
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





int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;
    char buffer[BUFFER_SIZE];

    // 서버 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        error("Error creating socket");
    }

    // 서버 주소 설정
    memset((void *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons((u_short)atoi(argv[1]));

    // 서버 소켓 바인딩
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("Error binding socket");
    }

    // 클라이언트의 연결 요청 대기
    if (listen(server_socket, SOMAXCONN) < 0) {
        error("Error listening on socket");
    }

    printf("File server started. Waiting for connections...\n");

    while (1) {
        // 클라이언트의 연결 수락
        client_address_length = sizeof(client_address);
        fflush(stdout); 
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_length);
        if (client_socket < 0) {
            error("Error accepting connection");
        }
        printf("Connection : Host IP %s, Port %d, socket %d\n",
	inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), client_socket);

        printf("Client connected.\n");
        FILE *fp = fdopen(client_socket, "r"); 
        buffer[0] = 0;
	if (!fgets(buffer, BUFSIZ - 1, fp)) {
	  printf("No Data: close connection");
	  fclose(fp);
	  if (close(client_socket)) perror("close error");
	    continue;
	}
        printf("%s",buffer);

        // 클라이언트로부터 명령어 수신
         // memset(buffer, 0, BUFFER_SIZE);
        char *command = strtok(buffer, " \t\n\r"); // Command
        char *filename = strtok(NULL, " \t\n\r");


        if (strcmp(command, "LS") == 0) {
            // 파일 목록 조회
            printf("Listing files...\n");
            list_files(client_socket,filename);
        } else if (strcmp(command, "GET") == 0) {
            // 파일 다운로드
            printf("Downloading file...\n");
            // 파일 전송
            send_file(client_socket, filename);
        } else if (strcmp(command, "PUT") == 0) {
            // 파일 업로드
            printf("Uploading file...\n");
            // 파일 수신
            receive_file(client_socket, filename);
        } else {
            // 잘못된 명령어 처리
            printf("Unknown command: %s\n", buffer);
        }
        // 클라이언트 연결 종료
        //close(client_socket);
        //printf("Client disconnected.\n");
    }

}

