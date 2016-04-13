#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_MSG_LEN 256
#define MAX_NAME_LEN 16

#define PUBLIC_PORT 6666

struct msg{
	char name[MAX_NAME_LEN];
	char time[32];
	char message[MAX_MSG_LEN];	 
};
typedef struct msg msg;

void initMsg(char* init_name,char* init_msg,msg* m) {
	strcpy(m->name,init_name);
	strcpy(m->message,init_msg);
	time_t now;
	time(&now);
	strcpy(m->time,asctime(gmtime(&now)));
}

void sendMsg(int client_fd,msg* m) {
	write(client_fd,m,MAX_MSG_LEN);
}

void recvMsg(int fd,msg* m) {
	read(fd, m, MAX_MSG_LEN);
}

void showMsg(msg* m) {
	printf("\n%s   %s",m->name,m->time);
	printf("%s\n",m->message);
}
