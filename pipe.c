#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PIPE_SUCCESS 0
#define MSG_LENGTH 512
#define BUF_SIZE 512+1

#define CHILD_EXIT 100

struct Message {
	int type;
	int length;
	char msg[BUF_SIZE];
};
typedef struct Message Message;


void initMsg(Message* init_msg,int t,int len,char str[]) {
	init_msg->type = t;
	init_msg->length = len;
	strcpy(init_msg->msg,str);
}


void showMsg(Message* recv_msg) {
	printf("message type : %d\n",recv_msg->type);
	printf("message length : %d\n",recv_msg->length);
	printf("message : %s\n",recv_msg->msg);
}


void readArgs(int* msg_type,int* msg_len,char buffer[],char* msg_to) {
	char type[10];
	memset(buffer,'\0',sizeof(buffer));
	do {
		printf("Please input the type of messge to %s(int):\n",msg_to);
		scanf("%s",type);
	}	
	while(checkType(type) == 0);
	getchar();
	*msg_type = transformType(type);
	printf("Please input message to %s. Press 'Enter' to send.(input \"q\" or \"Q\" to quit)\n",msg_to);
	fgets(buffer,BUF_SIZE,stdin);  //!!!!!!
	*msg_len = strlen(buffer)-1;
	buffer[*msg_len] = '\0';
}


int checkType(char type[]) {
	int i = 0;	
	for(;i<strlen(type);i++) {
		if(type[i] < '0' || type[i] > '9')
			return 0;
	}
	return 1;
}

int transformType(char type[]) {
	int i = 0;
	int num = 0;
	for(;i<strlen(type);i++) {
		num = num*10 + (type[i] - '0');
	}
	return num;
}

int needExit(Message recv_msg) {
	return ( strcmp(recv_msg.msg,"q") == 0 || strcmp(recv_msg.msg,"Q") == 0 );
}

void parentExit() {					
	int child_stat;
	pid_t child_pid = wait(&child_stat);
	if(WIFSTOPPED(child_stat)) {
		fprintf(stderr,"Child process exited abnormally\n");
	}
	printf("Parent process exit\n");
	exit(EXIT_SUCCESS);
}


int main() {
	int msg_type = 0;
	int msg_len = 0;
	int file_pipes_one[2];
	int file_pipes_two[2];
	char buffer[BUF_SIZE];
	Message send_msg;
	Message recv_msg;

	if(pipe(file_pipes_one) == PIPE_SUCCESS && pipe(file_pipes_two) == PIPE_SUCCESS) {
		pid_t child_pid = fork();
		switch(child_pid) {
			case -1 :
				perror("fork failure ");
				exit(EXIT_FAILURE);
					
			case 0 :
				while(1) {
					read(file_pipes_one[0], &recv_msg, sizeof(Message));					
					if(needExit(recv_msg)) {
						printf("\nChild process exit\n");
						exit(CHILD_EXIT);
					}

					printf("\nIn child process:\n");
					printf("receive message form parent\n");
					showMsg(&recv_msg);
					
					readArgs(&msg_type,&msg_len,buffer,"parent");
	
					initMsg(&send_msg,msg_type,msg_len,buffer);

					write(file_pipes_two[1], &send_msg, sizeof(Message));
					if(needExit(send_msg)) {
						printf("\nChild process exit\n");
						exit(CHILD_EXIT);
					}
				}
				break;

			default :
				while(1) {
					printf("\nIn parent process:\n");
					
					readArgs(&msg_type,&msg_len,buffer,"child");

					initMsg(&send_msg,msg_type,msg_len,buffer);
					write(file_pipes_one[1], &send_msg, sizeof(Message));
					if(needExit(send_msg)) {
						parentExit();
					}
					

					read(file_pipes_two[0], &recv_msg, sizeof(Message));					
					if(needExit(recv_msg)) {
						parentExit();
					}
					printf("\nIn parent process:\n");
					printf("receive message form child.\n");
					showMsg(&recv_msg);
					printf("\n");
				}
				break;
			}
	} else {
		perror("create pipe failure ");
		exit(EXIT_FAILURE);
	}
}
