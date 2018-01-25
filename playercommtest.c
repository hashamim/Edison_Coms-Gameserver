#include "player_comm.h"
#include <time.h>
#include <stdio.h>
#define BLUETAGSBLUE 0x10
#define BLUEBLOCKSBLUE 0X20

int main(int argc, char *argv[]){//webcam usage: a.out 'webcamportno' 'imuport'
	int webcam_socket_fd = hostConnection(argv[1]);
	int imu_socket_fd = hostConnection(argv[2]);
	printf("Connected!\n");
	char msg = BLUEBLOCKSBLUE;
	send(webcam_socket_fd,&msg,1,0);
	send(imu_socket_fd,&msg,1,0);
	
	sleep(2);
	msg = BLUETAGSBLUE;
	send(webcam_socket_fd,msg,1,0);
	send(imu_socket_fd,msg,1,0);
}