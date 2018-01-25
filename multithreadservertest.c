#include "game_server.h"
#include "player_comm.h"
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
int main(int argc, char *argv[]){ //main server code usage: a.out 'ip' 'webcamport' 'imuport'

	players[BLUE].webcam_socket_fd = connectToHost(argv[1],argv[2]);
  sleep(1);
	players[BLUE].imu_socket_fd = connectToHost(argv[1],argv[3]);
 printf("Connected!\n");
	struct pollfd webcampoll[2], imupoll[2];
	webcampoll[BLUE].fd = players[BLUE].webcam_socket_fd;
	webcampoll[RED].fd = players[RED].webcam_socket_fd;
	imupoll[BLUE].fd = players[BLUE].imu_socket_fd;
	imupoll[RED].fd = players[RED].imu_socket_fd;
	short events = POLLIN | POLLERR | POLLHUP;
	webcampoll[BLUE].events = events;
	webcampoll[RED].events = events;
	imupoll[BLUE].events = events;	imupoll[RED].events = events;
	
	char webcambuf[32], imubuf[32];
	char webcam, imu;
 while(1){
  	while(1){	
  		poll(webcampoll,2,0);
  		if(webcampoll[BLUE].revents & POLLIN == POLLIN){
  			read(webcampoll[BLUE].fd,webcambuf,32);
  			poll(imupoll+BLUE,2,200);
  			if(imupoll[BLUE].revents & POLLIN == POLLIN){
  				read(imupoll[BLUE].fd,imubuf,32);
  				webcam = webcambuf[0];
  				imu = imubuf[0];
  				if(webcam>>2 == imu>>2)
  					break;
  			}			
  		}
  		if(webcampoll[RED].revents & POLLIN == POLLIN){
  			read(webcampoll[RED].fd,webcambuf,32);
  			poll(imupoll+RED,2,200);
  			if(imupoll[RED].revents & POLLIN == POLLIN){
  				read(imupoll[RED].fd,imubuf,32);
  				webcam = webcambuf[0];
  				imu = imubuf[0];
  				if(webcam>>2 == imu>>2)
  				break;
  			}	
  		}
  	}
   if(webcampoll[BLUE].revents == POLLHUP
	printf("Webcam: %x, IMU: %x\n",webcam,imu); 
	}
  return 1;
}
