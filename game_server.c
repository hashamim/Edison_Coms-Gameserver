#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include "game_server.h"
#include <errno.h>


struct player players[NUM_PLAYERS];
volatile int player_done;
static char * player_colors[] = {"BLUE","RED","GREEN","YELLOW"};
static char * player_actions[] = {"IDLE","TAG","BLOCK"};

void playerAction(unsigned int actor, unsigned int target, unsigned int action){
	printf("Actor: %s, Action: %s, Target: %s\n",player_colors[actor],player_actions[action],player_colors[target]);
}

/*
void *imu_func(void *playerptr){
	struct player *ptr = (struct player *) playerptr;
	int imu_socket_fd = ptr->imu_socket_fd;
	char msg;
	unsigned int actor, action;
	while(1){	
		int recerr = recv(ptr->imu_socket_fd,&msg,1,0);
		if(recerr < 0){
			fprintf(stderr,"Error reading from webcam socket: %s\n",strerror(recerr));
		}
		printf("data received from imu: %x/n",msg);
		actor = ACTOR_MASK & msg;
		action = msg>>2;
		players[actor].imu_action = action;
		sleep(1); //change to millisecond sleep
		players[actor].imu_action = IDLE;
		msg = 0;
		if(player_done)
			break;
	}
	
}

void *playerFunction(void *playerptr){ //player thread functions read input from edisons
	//setup thread to collect data from IMU
	pthread_t imu_thread;
	int terr = pthread_create(&imu_thread,NULL,imu_func, playerptr);
	if(terr != 0){
			fprintf(stderr,"Error creating IMU_sensor thread: %s\n",strerror(terr));
			return;
		}
	
	//start reading from webcam in parallel with IMU
	struct player *Player = (struct player *) playerptr;
	char msg;
	unsigned int target, actor,action;
	int webcam_socket_fd = Player->webcam_socket_fd;
	while(1){
		int recerr = recv(webcam_socket_fd,&msg,1,0);
		if(recerr < 0){
			fprintf(stderr,"Error reading from webcam socket: %s\n",strerror(recerr));
		}
		printf("data received from webcam: %x\n",msg);
		actor = ACTOR_MASK & msg;
		action = msg>>4;
		target = (msg & TARGET_MASK)>>2;
		if(players[actor].imu_action == action)
			playerAction(actor,target,action);
		else{
			sleep(1);
			if(players[actor].imu_action == action)
				playerAction(actor,target,action);
		}
		players[actor].imu_action == IDLE;
		action = IDLE;
		msg = 0;
		if(player_done)
			break;
	}	
}

int initPlayers(pthread_t playerthreads[],int numplayers){
	int num_threads = numplayers;  //CHANGE THIS VALUE TO CHANGE NUMBER OF PLAYER THREADS
	int i;
	//initialize players
	for(i=0;i<num_threads;i++){
		int thr = pthread_create(&playerthreads[i],NULL,playerFunction,(void *) &players[i]);
		if(thr != 0){
			fprintf(stderr,"Error creating player thread: %s\n",strerror(thr));
			return -1;
		}
	}
		
}

*/
