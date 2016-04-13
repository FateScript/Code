#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "msg.h"


void removeClient(int fd,fd_set* fds) {
	close(fd);
	FD_CLR(fd, fds);
	printf("removing client on fd %d\n", fd);
}


int main(int argc, char * agrv[])
{
	int server_sockfd,client_sockfd,max_fd;
	int server_len;
	int result;	
	int fd,nread;

	struct sockaddr_in client_address;
	struct sockaddr_in server_address;
	
	
	fd_set readfds,temp_fds;

	msg message;
	
	//init varies
	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	max_fd = server_sockfd;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PUBLIC_PORT);
	server_len = sizeof(server_address);

	result = bind(server_sockfd,(struct sockaddr *)&server_address,server_len);
	if(result == -1)
	{
		perror("server error");
		exit(1);
	}		

	listen(server_sockfd,5);
	
	FD_ZERO(&readfds);
	FD_SET(server_sockfd,&readfds);

	while(1)
	{	
		printf("server waiting\n");
		temp_fds = readfds;

		result = select(max_fd + 1, &temp_fds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)0);
		
		switch (result) {	
		
		case -1:
			perror("select");
			exit(1);
		
		case 0:
			break;
		
		default: 
			
			for(fd = 0; fd<= max_fd ; fd++) {
				if(FD_ISSET(fd,&temp_fds)) {
					//connect 
					if(fd==server_sockfd) {	

						int client_len = sizeof(client_address);
						client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);	
						FD_SET(client_sockfd,&readfds);
						max_fd = (client_sockfd > max_fd)? client_sockfd:max_fd;
						printf("adding client on fd %d\n",client_sockfd);

					}else {
						ioctl(fd, FIONREAD, &nread);
						
						if(nread == 0) {
							//disconnect
							removeClient(fd, &readfds);
						}else {
							//serve normally
							recvMsg(fd, &message);

							if(strcmp(message.message, "q") == 0) {
								sendMsg(fd, &message);
								removeClient(fd, &readfds);
							}else {
								int tfd;
								for(tfd = server_sockfd+1; tfd <= max_fd; tfd++) {
									sendMsg(tfd, &message);
								}
							}	
						}
					}	
				}
			}
		}	 	
	}                	
	exit(0);         	
}     
