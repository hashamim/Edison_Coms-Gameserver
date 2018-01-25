#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

int hostConnection(char* portno){
	int listen_sock_fd, status;
	struct sockaddr_storage host_addr;
	struct addrinfo hints, *res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL,portno,&hints,&res);
	listen_sock_fd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(bind(listen_sock_fd,res->ai_addr,res->ai_addrlen) != 0 ){
		fprintf(stderr,"Error binding to Port\n");
		return -1;
	}
	int yes =  1;
	setsockopt(listen_sock_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	listen(listen_sock_fd,3);
	int hostlen = sizeof(host_addr);
	status = accept(listen_sock_fd, (struct sockaddr *) &host_addr,&hostlen);
	if(status < 0){
		fprintf(stderr,"Error accepting connection: %s\n",strerror(errno));
		return -1;
	}
	freeaddrinfo(res);
	return status;
}


int connectToHost(char* hostIP,char* portno){
	int player_sock_fd;
	struct addrinfo hints,*player;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int status = getaddrinfo(hostIP,portno,&hints,&player);
	if(status != 0){
		fprintf(stderr,"Getaddrinfo error: %s\n",gai_strerror(status));
		return -1;
	}
	player_sock_fd = socket(player->ai_family,player->ai_socktype,player->ai_protocol);
	if(player_sock_fd < 0){
		fprintf(stderr,"ERROR creating Socket: %d\n",errno);
		return -1;
	}
	int connerr = connect(player_sock_fd,player->ai_addr,player->ai_addrlen);
	if(connerr != 0){
		fprintf(stderr,"Error Connecting to Player: %d\n",errno);
		return -1;
	}
	freeaddrinfo(player);
	return player_sock_fd;
}

