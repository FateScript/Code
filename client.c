#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "msg.h"

#define FORK_FAILED -1
#define CHILD_PROC 0

#define EXIT_CODE 123


void waitChild() {
    int status;
    wait(&status,0); 
    if(WIFEXITED(status)) {
        printf("child exit\n");
    }else {
        printf("child exit abnormally!\n");
    }
}


void connectServer(int sockfd, struct sockaddr_in address) {
	int result;
	printf("now connect to the server\n");
	result = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
	if(result == -1) {
		perror("client error");
		exit(1);
	}else {
		printf("connect successfully!\n");
	}
}


int main() {

	int sockfd;
	char buffer[MAX_MSG_LEN];
	char name[MAX_NAME_LEN];

	struct sockaddr_in address;
	
	msg send_msg,recv_msg;
	pid_t child_proc;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(PUBLIC_PORT);
	
	connectServer(sockfd,address);
	
	child_proc = fork();
	switch (child_proc) {
		
	case FORK_FAILED:
		perror("fork failed");
		exit(1);
		
	case CHILD_PROC:
		while(1) {
			recvMsg(sockfd, &recv_msg);
			if(strcmp(recv_msg.message, "q") == 0) {
				exit(EXIT_CODE);
			}
			showMsg(&recv_msg);
		}

	default:	
		printf("input your name for chat:\n");
		fgets(name,MAX_NAME_LEN,stdin);
		name[(int)strlen(name)-1] = '\0';

		printf("now,input what you want to say,q to quit:\n");
		while(1) {			
			fgets(buffer, MAX_MSG_LEN, stdin);
			buffer[(int)strlen(buffer)-1] = '\0';
			
			initMsg(name,buffer,&send_msg);
			sendMsg(sockfd, &send_msg);
            if(strcmp(buffer,"q") == 0) {
                break;
            }
		}
        waitChild();
		close(sockfd);
		exit(0);
	}
	
}
