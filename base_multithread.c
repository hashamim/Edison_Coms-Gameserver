#include <time.h>
#include <pthread.h>
#include "game_server.h"


struct player players[NUM_PLAYERS];
volatile int player_done;
char * player_colors[] = {"BLUE","RED","GREEN","YELLOW"};
char * player_actions[] = {IDLE,TAG,BLOCK};
void *imu_func(void *playerptr){
	int imu_socket_fd = ((struct player *)playerptr->imu_socket_fd);
	char msg;
	uint actor, action;
	while(1){	
		int recerr = recv(Player->imu_socket_fd,&msg,1,0);
		if(recerr < 0){
			fprintf(stderr,"Error reading from webcam socket: %s\n",strerror(recerr));
		}
		actor = ACTOR_MASK & msg;
		action = msg>>2;
		players[actor].imu_action = action;
		nsleep(500000000);
		players[actor].imu_action = IDLE;
		
		
	}
	
}

void playerAction(uint actor, uint target, uint action){
	printf("Actor: %s, Action: %s, Target: %s\n",actor,target,actor);
}

void *playerFunction(void *playerptr){ //player thread functions read input from edisons
	//setup thread to collect data from IMU
	pthread_t imu_thread;
	int terr = pthread_create(&imu_thread,NULL,imu_func, playerptr);
	if(terr != 0){
			fprintf(stderr,"Error creating IMU_sensor thread: %s\n",strerror(terr));
			return -1;
		}
	
	//start reading from webcam in parallel with IMU
	struct player *Player = (struct player *) player;
	char msg;
	uint target, actor,action;
	int webcam_socket_fd = Player->webcacm_socket_fd;
	while(1){
		int recerr = recv(Player->webcam_socket_fd,&msg,1,0);
		if(recerr < 0){
			fprintf(stderr,"Error reading from webcam socket: %s\n",strerror(recerr));
		}
		actor = ACTOR_MASK & msg;
		action = msg>>4;
		target = (msg & TARGET_MASK)>>2;
		if(players[actor].imu_action == action)
			playerAction(actor,target,action);
		else{
			nsleep(400000000);
			if(players[actor].imu_action == action)
				playerAction(actor,target,action);
		}
		players[actor].imu_action == IDLE;
		action = IDLE;
		
	}
	
	
}

int initPlayers(pthread_t players[]){
	int num_threads = 2;  //CHANGE THIS VALUE TO CHANGE NUMBER OF PLAYER THREADS
	int i;
	//initialize players
	for(i=0;i<num_threads;i++){
		int thr = pthread_create(&players[i],NULL,playerFunction,(void *) &players[i]);
		if(thr != 0){
			fprintf(stderr,"Error creating player thread: %s\n",strerror(thr));
			return -1;
		}
	}
		
}

