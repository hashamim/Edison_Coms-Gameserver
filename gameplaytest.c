#include "player_comm.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
//#include <mraa/gpio.h>
#include <signal.h>

char* testServerIp = "52.37.92.22";
struct player{
	int webcam_socket_fd;
	int imu_socket_fd;
	char* webcam_ip;
	char* imu_ip;
};

//Display constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
//player colors
#define BLUE 0
#define RED 1
#define PURPLE 2
#define GREEN 3

//player actions
#define BLOCK 0
#define TAG 1
#define ESCAPE 2
#define SHOOT 3
#define REVIVE 4

#define NUMPLAYERS 3
#define BUTTON 3
struct player players[NUMPLAYERS];
static char * player_colors[] = {"BLUE","RED","PURPLE","GREEN"};
static char * player_actions[] = {"blocks","tags","escapes","shoots","revives"};
int player_blocking[NUMPLAYERS];
int player_numblocks[NUMPLAYERS];
int player_hp[NUMPLAYERS];
int player_doubledamage[NUMPLAYERS];
int numvictims;
int victimpts = 0;
int killerpts = 0;
int victimskilled = 0;
char msg;
int gameplaying;
//mraa_gpio_context button;

unsigned getAction(unsigned input)
{
return ( input & 15);


   //extract action bits
}

unsigned getPlayerId(unsigned input)
{
return ((input >>4) &3);

 //extract player id
}

unsigned getTargetId(unsigned input)
{
return ((input >>6) &3);
 //extract target id
}


void reset(){
//gameplay init
	int i = 0;
	while(i < NUMPLAYERS){
		player_blocking[i] = 0;
		if(player_numblocks[i] == 0){
		player_numblocks[i] = 1;} //players start with 1 block
		player_doubledamage[i] = 0;
		player_hp[i] = 1;
		i++;
	}
	player_numblocks[0] = 0;
	player_hp[0] = 3;
	numvictims = NUMPLAYERS - 1;
	gameplaying = 1;
	msg = 0;
	victimskilled = 0;
}
void reset_score(){
	victimpts = 0;
	killerpts = 0;
}

void playgame(int msg){
	unsigned action = getAction(msg);//extract action bits
	unsigned playerid = getPlayerId(msg);
	unsigned targetid = getTargetId(msg);
	if(msg == 18) //escape caused by error on player side fix player code and remove this
		action = 2;
	if (player_hp[playerid] != 0) //can't do anything if you're dead
		printf("Action: %s\n", player_actions[action]);
		printf("Player: %s\n", player_colors[playerid]);
		switch (action){
			case 0:
				if(player_numblocks[playerid] != 0){ //if player can block
					player_numblocks[playerid]--; //reduce number of blocks
					player_blocking[playerid] = 1;
					printf("%d blocking \n%d has %d blocks available/n", playerid,playerid,player_numblocks[playerid]); //player blocking
				} 
				if(player_numblocks[playerid]== 0)
					printf("%d is out of blocks!\n", playerid);
				break;
			case 3:
			case 1:
				printf("Target: %d\n", targetid);
				if (player_blocking[targetid] == 1 ){ //remove block
					player_blocking[targetid] = 0; 
					printf("%i blocked it!\n",targetid);
					break;
				}
				else{ //someone loses hp
					if(player_hp[targetid] == 0) //victim is dead/escaped
						break;
					if(player_doubledamage[playerid] == 1){//double damage
						player_hp[targetid]--; //lose 2hp if double dmg
						printf("Critical hit on %d!\n", targetid);
						player_doubledamage[playerid] = 0; 
					}		
					player_hp[targetid]--; //lose 1hp
					printf("%d has lost hp, now has %d hp\n", targetid,player_hp[targetid]);
					if (playerid == 0) {//give pts
						killerpts++;
						if(player_hp[playerid] < 3) //give killer hp back
						{
							player_hp[playerid]++;
							printf("Killer gained 1hp back, now has %d hp\n", player_hp[0]);
						}
						printf("Killer got 1 pt. Killer pts: %d\n", killerpts);
					}
					if(targetid == 0){ //target is killer, give victim 1 block
						player_numblocks[playerid]++;
						printf("%d has gained 1 block. %d blocks available\n", playerid,player_numblocks[playerid]);
					}
					if (player_hp[targetid] <= 0){//if target dies
						printf("%d has died\n", targetid);
						/*
						send msg to dead player
						*/
						if (targetid == 0){ //killer dies
							printf("Killer has died. Starting new round in 10s\n");
							reset(); //reset game state
							sleep(10);
							victimpts += 3;
							//  gameplaying = 0;
							/*
							send victory msg to victims
							*/
							break;
						}
						else{
							victimskilled++;
							numvictims--;
						if (numvictims == 0) //all victims dead
						{
							if(victimskilled == NUMPLAYERS - 1)
							{
								printf("All victims have died\n");

								gameplaying = 0;
								killerpts += 3; //add bonus pts

								printf("Killer points: %d\n", killerpts);
								printf("Victim points: %d\n", victimpts);
								if(killerpts > victimpts)
								printf("Killer wins!\n");
								else if(victimpts > killerpts)
								printf("Victims win!\n");
								else printf("Tie!\n");
								printf("Game starting again in 10s\n");
								reset();
								reset_score();
								sleep(10);
							}
							else{
								printf("No more victims in the round, starting new round in 10s\n");
								reset();
								sleep(10);
							}
						}
					}
				}
				break;

			}
			case 2: //escaping
				//if(mraa_gpio_read(button) == 1){
					printf("%d has escaped\n", playerid);
					player_hp[playerid] = 0;
					numvictims--;
					victimpts++;
				//}
				if(numvictims == 0){
					printf("All victims have escaped, starting new round in 10s\n");
					reset();
					sleep(10);
				}
				break;
			case 4: //reviving
				if(player_hp[targetid] == 0 && targetid != 0){ //target is not killer and is dead
					player_hp[targetid] = 1; //revive target;
					player_doubledamage[playerid] = 1; //give player double dmg
					printf("%d has been revived!\n", targetid);
					printf("%d has gained double damage on their next hit!\n", playerid);
					numvictims++;
					victimskilled--;
				}
		}
		//update display

}


               //                          0            1                   2               3            4               5             6               7            8
int main(int argc, char *argv[]){    //  ./gameplay  blue webcam ip  blue webcam port  blue imu ip  blue imu port  red webcam ip red webcam port red imu ip red imu port
	//intialize button for esapce
	//button = mraa_gpio_init(BUTTON); 
	//mraa_gpio_dir(button,MRAA_GPIO_IN);
	//new usage: ./a.out [number_of_players]

	//debug implementation
	/*char ipbuf[32];
	char portbuf[8];
	int numplayers = atoi(argv[1]);


	if(numplayers < 1 || numplayers > 4){
	printf("Must enter number between 1 and 4\n");
	return 1;
	}
	int i;
	for(i=0;i<numplayers;i++){
		//connect to webcam
		do{
		  printf("Enter webcam ip for %s: ",player_colors[i]);
		  scanf("%31s",ipbuf);
		  players[i].webcam_ip = strdup(ipbuf);
		  printf("Enter webcam port number for %s: ", player_colors[i]);
		  scanf("%7s",portbuf);
		  players[i].webcam_socket_fd = connectToHost(players[i].webcam_ip,&portbuf[0]);
		  if(players[i].webcam_socket_fd < 0)
		    printf("Unsuccessfully connected to %s webcam\n", player_colors[i]);
		}while(players[i].webcam_socket_fd < 0);
		//connect to imu
		do{
		  printf("Enter imu ip for %s: ",player_colors[i]);
		  scanf("%31s",ipbuf);
		  players[i].imu_ip = strdup(ipbuf);
		  printf("Enter imu port number for %s: ", player_colors[i]);
		  scanf("%7s",portbuf);
		  players[i].imu_socket_fd = connectToHost(players[i].webcam_ip,&portbuf[0]);
		  if(players[i].imu_socket_fd < 0)
		    printf("Unsuccessfully connected to %s imu\n", player_colors[i]);
		}while(players[i].imu_socket_fd < 0);
	}
	*/
	players[BLUE].webcam_socket_fd = connectToHost(testServerIp,argv[1]);
	sleep(1);
	printf("blue web\n");
	players[BLUE].imu_socket_fd = connectToHost(testServerIp,argv[2]);
	sleep(1);
	printf("blue imu\n");
	players[RED].webcam_socket_fd = connectToHost(testServerIp,argv[3]);
	sleep(1);
	printf("red web\n");
	players[RED].imu_socket_fd = connectToHost(testServerIp,argv[4]);
	sleep(1);
	printf("red imu\n");
	players[PURPLE].webcam_socket_fd = connectToHost(testServerIp,argv[5]);
	sleep(1);
	printf("purple web\n");
	players[PURPLE].imu_socket_fd = connectToHost(testServerIp,argv[6]);
	printf("Connected!\n");

	//poll struct initialization
	struct pollfd webcampoll[3], imupoll[3];
	webcampoll[BLUE].fd = players[BLUE].webcam_socket_fd;
	webcampoll[RED].fd = players[RED].webcam_socket_fd;
	webcampoll[PURPLE].fd = players[PURPLE].webcam_socket_fd;
	imupoll[BLUE].fd = players[BLUE].imu_socket_fd;
	imupoll[RED].fd = players[RED].imu_socket_fd;
	imupoll[PURPLE].fd = players[PURPLE].imu_socket_fd;

	short events = POLLIN | POLLERR | POLLHUP;
	webcampoll[BLUE].events = events;
	webcampoll[RED].events = events;
	webcampoll[PURPLE].events = events;
	imupoll[BLUE].events = events;	imupoll[RED].events = events;
	imupoll[RED].events = events;
	imupoll[PURPLE].events = events;

	char webcambuf[32], imubuf[32];
	char webcam, imu;
	int eof;
	reset(); //initialize game
	reset_score();

	//test inputs
	// unsigned char inputs[] = {16, 65, 18, 17, 17, 17, 65, '\0'};
	// int k =0;
	printf("Game starting\n");
	while(gameplaying==1){
		//polling loop
		while(1){	
			poll(webcampoll,3,0);
			if(webcampoll[BLUE].revents & POLLIN == POLLIN){
				eof = recv(webcampoll[BLUE].fd,webcambuf,32,0);
				if(eof <= 0)
	       			break;
	    		printf("BLUE WEBCAM: %d\n", webcambuf[0]);
	   			if(getAction((int ) webcambuf[0]) == 3){
					webcam = webcambuf[0];
					break; //shoot
				}
				poll(imupoll+BLUE,1,1000);
				if(imupoll[BLUE].revents & POLLIN == POLLIN){
					eof = recv(imupoll[BLUE].fd,imubuf,32,0);
					if(eof <= 0)
		 				break;
					imu = imubuf[eof-1];
					printf("BLUE IMU: %d\n", imu);
					webcam = webcambuf[0];
					if((webcam &15) == (imu & 15))
						break;
				}			
			}
			if(webcampoll[RED].revents & POLLIN == POLLIN){
				eof = recv(webcampoll[RED].fd,webcambuf,32,0);
				if(eof <= 0)
	       			break;
	    			printf("RED WEBCAM: %d\n", webcambuf[0]);
	   			if(getAction((int ) webcambuf[0]) == 2){
					webcam = webcambuf[0];
					break; //escape
				}
				poll(imupoll+RED,1,1000);
				if(imupoll[RED].revents & POLLIN == POLLIN){
					eof = recv(imupoll[RED].fd,imubuf,32,0);
					if(eof <= 0)
		 				break;
					imu = imubuf[eof-1];
					printf("RED IMU: %d\n", imu);
					webcam = webcambuf[0];
					if((webcam &15) == (imu & 15))
						break;
				}			
			}
			if(webcampoll[PURPLE].revents & POLLIN == POLLIN){
				eof = recv(webcampoll[PURPLE].fd,webcambuf,32,0);
				if(eof <= 0)
	       			break;
	    			printf("PURPLE WEBCAM: %d\n", webcambuf[0]);
	   			if(getAction((int ) webcambuf[0]) == 2){
					webcam = webcambuf[0];
					break; //escape = 2
				}
				poll(imupoll+PURPLE,1,1000);
				if(imupoll[PURPLE].revents & POLLIN == POLLIN){
					eof = recv(imupoll[PURPLE].fd,imubuf,32,0);
					if(eof <= 0)
		 				break;
					imu = imubuf[eof-1];
					printf("PURPLE IMU: %d\n", imu);
					webcam = webcambuf[0];
					if((webcam &15) == (imu & 15))
						break;
				}
			}

		}

		if(eof <= 0)
		break;
		//	printf("Webcam: %x, IMU: %x\n",webcam,imu); 
		playgame((int)webcam);
		//playgame((int)imu);

	  

	}
	return 0;
}
