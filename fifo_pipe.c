#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PIPE_SUCCESS 0
#define MSG_LENGTH 512
#define BUF_SIZE 512+1

#define CHILD_EXIT 100
#define FIRST_FIFO_NAME  "/tmp/first_fifo"
#define SECOND_FIFO_NAME  "/tmp/second_fifo"


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
	fgets(buffer,BUF_SIZE,stdin);
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
	int pipe_fd_one,pipe_fd_two;
	int first_res,second_res;
	char buffer[BUF_SIZE];
	Message send_msg;
	Message recv_msg;

	if(access(FIRST_FIFO_NAME,F_OK) == -1 || access(SECOND_FIFO_NAME,F_OK) == -1) {
		first_res = mkfifo(FIRST_FIFO_NAME,0777);
		if(first_res != 0) {
			fprintf(stderr,"Could not create fifo %s\n",FIRST_FIFO_NAME);
			exit(EXIT_FAILURE);
		}
		second_res = mkfifo(SECOND_FIFO_NAME,0777);
		if(second_res != 0) {
			fprintf(stderr,"Could not create fifo %s\n",SECOND_FIFO_NAME);
			exit(EXIT_FAILURE);
		}
	}
	pid_t child_pid = fork();
	switch(child_pid) {
		case -1 :
			perror("fork failure ");
			exit(EXIT_FAILURE);
					
		case 0 :
			pipe_fd_one = open(FIRST_FIFO_NAME,O_WRONLY);
			pipe_fd_two = open(SECOND_FIFO_NAME,O_RDONLY);
			if(pipe_fd_one != -1 && pipe_fd_two != -1) {
				while(1) {
					read(pipe_fd_two, &recv_msg, sizeof(Message));					
					if(needExit(recv_msg)) {
						printf("\nChild process exit\n");
						exit(CHILD_EXIT);
					}

					printf("\nIn child process:\n");
					printf("receive message form parent\n");
					showMsg(&recv_msg);
					
					readArgs(&msg_type,&msg_len,buffer,"parent");
	
					initMsg(&send_msg,msg_type,msg_len,buffer);

					write(pipe_fd_one, &send_msg, sizeof(Message));
					if(needExit(send_msg)) {
						printf("\nChild process exit\n");
						exit(CHILD_EXIT);
					}
				}
				(void)close(pipe_fd_one);
				(void)close(pipe_fd_two); 
			}else {
				fprintf(stderr,"FIFO open failed in child process\n");
				exit(EXIT_FAILURE);
			}
			break;

		default :
			pipe_fd_one = open(FIRST_FIFO_NAME,O_RDONLY);
			pipe_fd_two = open(SECOND_FIFO_NAME,O_WRONLY);
			if(pipe_fd_one != -1 && pipe_fd_two != -1) {
				while(1) {
					printf("\nIn parent process:\n");
					
					readArgs(&msg_type,&msg_len,buffer,"child");

					initMsg(&send_msg,msg_type,msg_len,buffer);

					write(pipe_fd_two, &send_msg, sizeof(Message));
					if(needExit(send_msg)) {
						parentExit();
					}
					

					read(pipe_fd_one, &recv_msg, sizeof(Message));					
					if(needExit(recv_msg)) {
						parentExit();
					}
					printf("\nIn parent process:\n");
					printf("receive message form child.\n");
					showMsg(&recv_msg);
					printf("\n");
				}
				(void)close(pipe_fd_one);
				(void)close(pipe_fd_two);
			}else {
				fprintf(stderr,"FIFO open failed in parent process\n");
				exit(EXIT_FAILURE);
			}
			break;
	}
}
