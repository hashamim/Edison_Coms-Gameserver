#include "player_comm.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

//player colors
#define BLUE 0
#define RED 1
#define PURPLE 2
#define GREEN 3

#define BLUEMASK 0x00
#define REDMASK 0x10
#define PURPLEMASK 0x20
#define GREENMASK 0x30

//actions
#define BLOCK 0
#define TAG 1
#define ESCAPE 2
#define SHOOT 3
#define REVIVE 4

#define NOTARGET 0x3f
/*
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
*/

char msgbuf[64];                    // 0                1             2            3            4                5             6
int main(int argc, char * argv[]){ //usage bluewebcamfdportno blueimufdportno redwebcamfdportno redimufdportno purplewebcamfdportno  purpleimufdportno
	int webcamfd[3];
	int imufd[3];
	webcamfd[BLUE] = hostConnection(argv[1]);//open("blue.txt",O_WRONLY|O_CREAT, 0660);
	imufd[BLUE] = hostConnection(argv[2]);//open("blue.txt",O_WRONLY|O_CREAT, 0660);
	webcamfd[RED] = hostConnection(argv[3]);//open("red.txt",O_WRONLY|O_CREAT, 0660); 
	imufd[RED] = hostConnection(argv[4]);//open("red.txt",O_WRONLY|O_CREAT, 0660);
	webcamfd[PURPLE] = hostConnection(argv[5]);//open("purp.txt",O_WRONLY|O_CREAT, 0660);
	imufd[PURPLE] = hostConnection(argv[6]);//open("purp.txt",O_WRONLY|O_CREAT, 0660); 
	
	char player;
	char target;
	char action[8];
	uint8_t msg;
  printf("To send messages through sockets use format:\n'[player] [target] [action]'\nwhere player and target are chars e.g:\nb r tag\nblue tags red\n");
	while(1){ //usage: [color (lowercase)] [action (e.g tags, blocks)] [color]
		scanf(" %c %c %7s",&player,&target,action);
   //printf("Player: %c, target: %c action: %s\n", player, target, action);
     msg = 0;
		switch(target){
			case 'r':
				msg = REDMASK<<2;
				break;
			case 'p':
				msg = PURPLEMASK<<2;
				break;
			case 'g':
				msg = GREENMASK<<2;
				break;
			case 'b':
				msg = BLUEMASK<<2;
		}
		switch(action[0]){
			case 's':
				msg |= SHOOT;
				break;
			case 'r':
				msg |= REVIVE;
				break;
			case 'e':
				msg |= ESCAPE;
				msg &= NOTARGET;
				break;
			case 'b':
				msg |= BLOCK;
				msg &= NOTARGET;
				break;
			case 't':
				msg |= TAG;
		}
		switch(player){
			case 'r':
				msg |= REDMASK;
				write(webcamfd[RED],&msg,1);
				write(imufd[RED],&msg,1);
				break;
			case 'p':
				msg |= PURPLEMASK;
				write(webcamfd[PURPLE],&msg,1);
				write(imufd[PURPLE],&msg,1);
				break;
			case 'g':
				msg |= GREENMASK;

				break;
			case 'b':
				msg |= BLUEMASK;
        //printf("Hex: %x\n", msg);
				write(webcamfd[BLUE],&msg,1);
				write(imufd[BLUE],&msg,1);
		}
		
		
	}
	return 0;
}