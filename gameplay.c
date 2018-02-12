#include "player_comm.h"
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <mraa/gpio.h>
#include <signal.h>

             /*
  Players:
  0: killer
  1 - (NUMPLAYERS - 1): victims

  Actions:
  0: blocking
  1: tag
  2: escape
  3: shoot
  4: revive
*/

struct player{
	int webcam_socket_fd;
	int imu_socket_fd;
	char* webcam_ip;
	char* imu_ip;
};

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
static char * player_colors[] = {"BLUE","RED","GREEN","YELLOW"};
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
int action;
int playerid;
int targetid;
int gameplaying;
mraa_gpio_context button;
  

void reset()
{
//gameplay init
int i = 0;
while(i < NUMPLAYERS)
{

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
msg =0;
victimskilled =0;
}
void reset_score()
{
victimpts = 0;
killerpts = 0;

}

void playgame(int msg)
{
 action = getAction(msg);//extract action bits
    playerid = getPlayerId(msg);
    
    if(msg == 18) //escape
    action = 2;
    printf("Action: %d\n", action);
    printf("Playerid: %d\n", playerid);
    if (player_hp[playerid] != 0) //can't do anything if you're dead
      switch (action)
    {
        case 0:
	   if(player_numblocks[playerid] != 0) //if player can block
	{
	  player_numblocks[playerid]--; //reduce number of blocks
        player_blocking[playerid] = 1;
        printf("%d blocking \n", playerid); //player blocking
	  printf("%d has", playerid);
	  printf("%d blocks available \n", player_numblocks[playerid]);
     } 
	if(player_numblocks[playerid]== 0)
{
printf("%d is out of blocks!\n", playerid);
}
	  
	    break;
	   case 3:
        case 1:
          targetid = getTargetId(msg); //locate tagged target
          printf("Target: %d\n", targetid);
          if (player_blocking[targetid] == 1 ) //remove block
          { player_blocking[targetid] = 0; 
            break;
          }
          else //someone loses hp
          {
            if (player_hp[targetid] == 0) //victim is dead/escaped
              break;
		if(player_doubledamage[playerid] == 1)//double damage
		{ player_hp[targetid]--; //lose 2hp if double dmg
		printf("Critical hit on %d!\n", targetid);
		player_doubledamage[playerid] = 0; 
}		
            player_hp[targetid]--; //lose 1hp
            printf("%d has lost hp, now has", targetid);
		printf("%d hp\n", player_hp[targetid]);
            if (playerid == 0) //give pts
              {
              killerpts++;
		 if(player_hp[playerid] < 3) //give killer hp back
{
			player_hp[playerid]++;
printf("Killer gained 1hp back, now has %d hp\n", player_hp[0]);
}
             printf("Killer got 1 pt. Killer pts: %d\n", killerpts);
              }
          	 if(targetid == 0) //target is killer, give victim 1 block
		{
		player_numblocks[playerid]++;
		printf("%d has gained 1 block. ", playerid);
		printf("%d blocks available\n", player_numblocks[playerid]);
		}
            if (player_hp[targetid] <= 0) //if target dies
            {
            printf("%d has died\n", targetid);
              /*
                send msg to dead player
              */
              if (targetid == 0) //killer dies
              {
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
              else
              {
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
else
{
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
        if(mraa_gpio_read(button) == 1)
        {
        printf("%d has escaped\n", playerid);
          player_hp[playerid] = 0;
          numvictims--;
          victimpts++;
        }
          if(numvictims == 0)
          {
          printf("All victims have escaped, starting new round in 10s\n");
          reset();
          sleep(10);
          }
     	break;
	case 4: //reviving
	if(player_hp[targetid] == 0 && targetid != 0) //target is not killer and is dead
	{
	player_hp[targetid] = 1; //revive target;
	player_doubledamage[playerid] = 1; //give player double dmg
	printf("%d has been revived!\n", targetid);
	printf("%d has gained double damage on their next hit!\n", playerid);
	numvictims++;
	victimskilled--;
	}
      };
}
int getAction(int input)
{
return ( input & 15);


   //extract action bits
}

int getPlayerId(int input)
{
return ((input >>4) &3);

 //extract player id
}

int getTargetId(int input)
{
return ((input >>6) &3);
 //extract target id
}

               //                          0            1                   2               3            4               5             6               7            8
int main(int argc, char *argv[]){    //  ./gameplay  blue webcam ip  blue webcam port  blue imu ip  blue imu port  red webcam ip red webcam port red imu ip red imu port
  //intialize button for esapce

	button = mraa_gpio_init(BUTTON); 
 mraa_gpio_dir(button,MRAA_GPIO_IN);
//main server code usage: a.out 'webcam ip' 'imu ip' webcam port' 'imu port'

	players[BLUE].webcam_socket_fd = connectToHost(argv[1],argv[2]);
  printf("connected to ip: %s with portno %s\n",argv[1], argv[2]);
  sleep(1);
	players[BLUE].imu_socket_fd = connectToHost(argv[3],argv[4]);
 printf("connected to ip: %s with portno %s\n",argv[3], argv[4]);
 	players[RED].webcam_socket_fd = connectToHost(argv[5],argv[6]);
  printf("connected to ip: %s with portno %s\n",argv[5], argv[6]);
  sleep(1);
	players[RED].imu_socket_fd = connectToHost(argv[7],argv[8]);
 printf("connected to ip: %s with portno %s\n",argv[7], argv[8]);

	players[PURPLE].webcam_socket_fd = connectToHost(argv[9],argv[10]);
printf("connected to ip: %s with portno %s \n", argv[9], argv[10]);
	players[PURPLE].imu_socket_fd = connectToHost(argv[11],argv[12]);
printf("connected to ip: %s with portno %s\n",argv[11], argv[12]);
printf("Connected!\n");

	struct pollfd webcampoll[2], imupoll[2];
	webcampoll[BLUE].fd = players[BLUE].webcam_socket_fd;
	webcampoll[RED].fd = players[RED].webcam_socket_fd;
	webcampoll[PURPLE].fd = players[PURPLE].webcam_socket_fd;
	imupoll[BLUE].fd = players[BLUE].imu_socket_fd;
	imupoll[RED].fd = players[RED].imu_socket_fd;
	imupoll[PURPLE].fd = players[RED].imu_socket_fd;

	short events = POLLIN | POLLERR | POLLHUP;
	webcampoll[BLUE].events = events;
	webcampoll[RED].events = events;
	webcampoll[PURPLE].events = events;
	imupoll[BLUE].events = events;	imupoll[RED].events = events;
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
  	while(1){	
   
  		poll(webcampoll,2,0);
  		if(webcampoll[BLUE].revents & POLLIN == POLLIN){
  			eof = recv(webcampoll[BLUE].fd,webcambuf,32,0);
        if(eof <= 0)
           break;
        printf("BLUE WEBCAM: %d\n", webcambuf[0]);
	   if(getAction((int ) webcambuf[0]) == 3)
		{
webcam = webcambuf[0];
break; //shoot
}
  			poll(imupoll+BLUE,1,1000);
  			if(imupoll[BLUE].revents & POLLIN == POLLIN){
  				eof = recv(imupoll[BLUE].fd,imubuf,32,0);
          if(eof <= 0)
            break;
          printf("BLUE IMU: %d\n", imubuf[0]);
  				webcam = webcambuf[0];
  				imu = imubuf[0];
          if((webcam&15) == 3)
          break; //shoot = 3
  				if((webcam &15) == (imu & 15))
  					break;
  			}			
  		}
int i = 1;
while(i < 3)
{
  		if(webcampoll[i].revents & POLLIN == POLLIN){
				eof = recv(webcampoll[i].fd,webcambuf,32,0);
        if(eof <= 0)
           break;
	if(i == 1)
        printf("RED WEBCAM: %d\n", webcambuf[0]);
	else printf("PURPLE WEBCAM %d\n", webcambuf[0]);
        if(getAction((int) webcambuf[0]) == 2)
        {
        webcam = webcambuf[0];
        break; //escape
  			}
        poll(imupoll+i,1,1000);
  			if(imupoll[1].revents & POLLIN == POLLIN){
  				eof = recv(imupoll[RED].fd,imubuf,32,0);
          if(eof <= 0)
            break;
	if(i == 1)
          printf("RED IMU: %d\n", imubuf[0]);
	else printf("PURPLE	 IMU: %d\n", imubuf[0]);
  				webcam = webcambuf[0];
  				imu = imubuf[0];
	    int temp = getAction((int)webcam);
          if(temp == 2 || temp == 4)
          break;
  				if((webcam&15) == (imu&15))
  				break;
  			}	
  		}
i++;
}
  	}
   
   if(eof <= 0)
    break;
//	printf("Webcam: %x, IMU: %x\n",webcam,imu); 
    //printf("Input: ");
    //int input =  scanf("%d", &input);



    playgame((int)webcam);
    //playgame((int)imu);
   
      
  
	}
  return 1;
}


