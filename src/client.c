/**
 * Simple File Transfer Client For 2019 Introduction to Computer Network Lab3.
 * Author: Kerwin Tsai 
 * 2019/10
 * **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  /* contains a number of basic derived types */
#include <sys/socket.h> /* provide functions and data structures of socket */
#include <arpa/inet.h>  /* convert internet address and dotted-decimal notation */
#include <netinet/in.h> /* provide constants and structures needed for internet domain addresses*/
#include <unistd.h>     /* `read()` and `write()` functions */
#include <dirent.h>     /* format of directory entries */
#include <sys/stat.h>   /* stat information about files attributes */

#define MAX_SIZE 2048

void connection_handler (int sockfd);
void file_download_handler(int sockfd, char filename[]);

int main (int argc, char *argv[]) {
	int cli_fd;                   /* descriptor of client, used by `socket()` */
	struct sockaddr_in svr_addr;  /* address of server, used by `connect()` */

	/* create the socket, use `socket()` */
	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd < 0) {
		perror("Create socket failed.");
		exit(1);
	}

	
    if(argv[1] == '\0' || argv[2] == '\0'){
        printf("usage: client <Server IP> <Port>\n");
        exit(1);
    }

    int Port = atoi(argv[2]);

	bzero(&svr_addr, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(Port);
	if (inet_pton(AF_INET, argv[1], &svr_addr.sin_addr) <= 0) {
		perror("Address converting fail with wrong address argument");
		return 0;
	}


	/* connect to server with prepared sockaddr_in structure */
	if (connect(cli_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
		perror("Connect failed");
		exit(1);
	}

	printf("Server connected\n");

	connection_handler(cli_fd);

	close(cli_fd);
	return 0;
}

void connection_handler (int sockfd) {
    char filename[MAX_SIZE];  /* file want to download */
    char path[MAX_SIZE];      /* file path */
    char buf[MAX_SIZE];       /* store data from server */

    /* create download dir */
    sprintf(path, "./download");
    mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);

    /* read hello msg from server */
    memset(buf, '\0', MAX_SIZE);
    read(sockfd, buf, MAX_SIZE);
    printf("%s", buf);

    char tmp;
    int buflen;
    memset(buf, '\0', MAX_SIZE);
    while(1){
		int read_bytes = read(sockfd, &tmp, 1);
		if(read_bytes < 0) perror("ERROR reading from socket");
		
		if(tmp == 'e') break;
		else if(tmp != 's') continue;
		buflen = 0;
		while(1){
			int write_bytes = read(sockfd, &tmp, 1);
			if (write_bytes < 0) perror("ERROR reading from socket");

			if (tmp == 'f')
				break;

			buf[buflen] = tmp;
			++buflen;
		}
		printf("%*.*s\n", buflen, buflen, buf);
    }

    printf("-----------\nEnter the filename: ");
    while (scanf(" %s", filename) > 0) {
        if (strcmp(filename, ".exit") == 0) {
            break;
        }

        int write_bytes = write(sockfd, filename, strlen(filename));
        if(write_bytes < 0) {
            perror("Client write file name Failed");
            exit(1);
        }  

        /* download this file */
        file_download_handler(sockfd, filename);

        /* send next file request*/
        printf("-----------\nEnter the filename: ");
    }
    printf("[x] Socket closed\n");
}

void file_download_handler(int sockfd, char filename[]) {
	char buf[MAX_SIZE];   /* store data from server */
	char path[MAX_SIZE];  /* file path */

	int file_size = 0;    /*  size of this file */
	int read_byte = 0;    /*  bytes this time recv */
	int read_sum = 0;     /*  bytes have been recv */
	FILE *fp;

	/* receive start message from server */
	memset(buf, '\0', MAX_SIZE);
	read(sockfd, buf, MAX_SIZE);
	printf("%s", buf);

	/* receive file size */
	memset(buf, '\0', MAX_SIZE);
	read(sockfd, buf, MAX_SIZE);
	file_size = atoi(buf);

	/* file path */
	memset(path, '\0', MAX_SIZE);
	sprintf(path, "./download/%s", filename);

	read_sum = 0;
	fp = fopen(path, "wb");
  	if (fp) {
		while (read_sum < file_size) {
			memset(buf, '\0', MAX_SIZE);

			read_byte = read(sockfd, buf, MAX_SIZE);

			/* write file to local disk*/
			fwrite(&buf, sizeof(char), read_byte, fp);
			printf("%d ... ", read_byte);
			read_sum += read_byte;
		}
		putchar('\n');
		fclose(fp);

		/* receive download complete message */
		memset(buf, '\0', MAX_SIZE);
		read(sockfd, buf, MAX_SIZE);
		printf("%s", buf);
	} else {
		perror("Allocate memory fail\n");
		return;
	}
}
