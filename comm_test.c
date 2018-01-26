#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "player_comm.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#define SERVER "server"
#define CLIENT "client"

//usage: user ip(not used for player) port_number
int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Usage: ./comm_test [user] [ip] [port number]\n");
		return 0;
	}
	if(strcmp(argv[1],CLIENT)==0){ //client code receives data
    							int recerr;
		int base_sock_fd = connectToHost(argv[2],argv[3]);	
		char msg = 'a';
		while(1){
			recerr = recv(base_sock_fd,&msg,1,0);
			if(recerr <= 0){
				printf("recv: %d errno: %d\n", recerr, errno);
			       	break;
			}
      printf("%d\n",msg);
		}
	}
	else if(strcmp(argv[1],SERVER)==0){ //server code sends data

		int player_sock_fd = hostConnection(argv[3]);	
		char msg = 'a';
		int senderr;
		while(1){
			senderr = send(player_sock_fd,&msg,1,0);
			if(senderr <= 0){
				printf("send: %d errno: %d\n", senderr, errno);
				break;
			}
			sleep(1);
			msg++;
		}
	}
	else{
		fprintf(stderr,"Error in comm function, 'server' or 'client' not specified\n");
		return -1;
	}
 return  0;
}
