#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "base_comm.h"
#include <string.h>
#include "player_comm.h"
#include <unistd.h>
#include <stdio.h>
#include <mraa/i2c.h>
#include "LSM9DS0.h"
#include "MadgwickAHRS.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define BASE_COMM "base"
#define PLAYER_COMM "player"

#define degToRad 3.14159265359/180.f //would be faster as a constant

#define PLAYERID 3 //0-3, 2 bits, left shift by 6 bits
#define BLOCK 0
#define TAG 1

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
//comms variables
int message = 0;

//min's accelerometer drifts for 10Hz
double driftx = -0.0312;
double drifty = -0.2233;

//incrementing flags

//number of consecutive tag or block motions read before verification
int tagnum = 0;
int blocknum = 0;

//boolean to send to AP
int tagging = 0;
int blocking = 0;

//prevent msg from resending on accident with extended motions
int tagsent= 0;
int blocksent = 0;

int MIN_VALID_GESTURES = 5; //minimum valid motions to determine gesture
int tagthreshold = -200;
int blockthreshold = 200;
int noise = 150;

struct Angle
{
	double x;
	double y;
	double z;
}Omega,current_data;

#define microSeconds   10000//100000 //0.1s or 10Hz

const double time_interval = (double)microSeconds/1000000;

//usage: user ip(not used for player) port_number
int main(int argc, char* argv[]){
	if(strcmp(argv[1],BASE_COMM)==0){ //base_comm
		int base_sock_fd = connectToPlayer(argv[2],argv[3]);	
		char msg = 'a';
		while(1){
			recv(base_sock_fd,&msg,1,0);
			printf("%c\n",msg);
			int messagerec = msg;
			int gesture = msg%2;
			int player = msg>>1;
			printf("%d\n", gesture);
			printf("%d\n", player);
//			sleep(2);
		}
	}
	if(strcmp(argv[1],PLAYER_COMM)==0){ //player_comm
	
//initialize gyro variables
	data_t accel_data, gyro_data, mag_data;
	data_t gyro_offset;
	int16_t temperature;
	float a_res, g_res, m_res;
	mraa_i2c_context accel, gyro, mag;
	accel_scale_t a_scale = A_SCALE_4G;
	gyro_scale_t g_scale = G_SCALE_2000DPS;
	mag_scale_t m_scale = M_SCALE_2GS;

	//initialize Omega to zero and prev_data
	Omega.x = 0;
	Omega.y = 0;
	Omega.z = 0;
	//prev_data.x = 0;
	//prev_data.y = 0;
	//prev_data.z = 0;

	//initialize sensors, set scale, and calculate resolution.
	accel = accel_init();
	set_accel_scale(accel, a_scale);	
	a_res = calc_accel_res(a_scale);
	
	gyro = gyro_init();
	set_gyro_scale(gyro, g_scale);
	g_res = calc_gyro_res(g_scale);
	
	mag = mag_init();
	set_mag_scale(mag, m_scale);
	m_res = calc_mag_res(m_scale);

	//find offset for the gyro sensor
	gyro_offset = calc_gyro_offset(gyro, g_res);
	

	int player_sock_fd = connectToBase(argv[3]);	
	char msg = '0';
		
			while(1){

			
		accel_data = read_accel(accel, a_res);
		gyro_data = read_gyro(gyro, g_res);

		//calculates the angular rate of change in degrees per second
		current_data.x = gyro_data.x-gyro_offset.x;
		current_data.y = gyro_data.y-gyro_offset.y;
		current_data.z = gyro_data.z-gyro_offset.z;
		
		//convert angular rate of change to radians per second
		current_data.x *= degToRad;
		current_data.y *= degToRad;
		current_data.z *= degToRad;
 // current_data.x += driftx;
  //current_data.y +=drifty;
		//perform the Madgwick Algorithm
		//with the Madgwick algorithm, in MadgwickAHRS.c
		//you will need to change the variable sampleFreq to the frequency that you are reading data in at
		//this is currently set to 10Hz
		MadgwickAHRSupdateIMU(current_data.x,current_data.y,current_data.z,accel_data.x,accel_data.y,accel_data.z);

		//convert the quaternion representation to Euler Angles in radians
		Omega.x = atan2(2*(q2*q3-q0*q1),2*q0*q0+2*q3*q3-1);
		Omega.y =-1*asin((double)(2*(q0*q2+q1*q3)));
		Omega.z = atan2(2*(q1*q2-q0*q3),2*q0*q0+2*q1*q1-1); 

		//convert Angles from  radians to degrees
		Omega.x *= 180 / 3.14159265359;
		Omega.y *= 180 / 3.14159265359;
		Omega.z *= 180 / 3.14159265359;
 

   if((gyro_data.z- gyro_offset.z)<= tagthreshold && abs(gyro_data.x - gyro_offset.x) <=noise && abs(gyro_data.y -gyro_offset.y) <=noise)
   {
   tagnum++;
   if(tagnum == MIN_VALID_GESTURES && tagsent ==0)
   {
   tagsent = 1;
   tagging = 1; //will use this flag for communication purposes
   printf("TAGGING\n");
   
   message |= (PLAYERID << 1);
   message |= TAG;
  	
   tagging = 0;
   tagnum = 0;
   }
   }
   else
   {
   tagsent = 0;
   tagnum = 0;
   }
   
   //blocking
      if((gyro_data.y-gyro_offset.y) >= blockthreshold && abs(gyro_data.x-gyro_offset.x) <=noise && abs(gyro_data.z-gyro_offset.z) <=noise)
   {
   blocknum++;
   if(blocknum == MIN_VALID_GESTURES &&blocksent == 0)
   {
   blocksent = 1;
   blocking = 1; //will use this flag for communication purposes
   printf("BLOCKING\n");
   
   message |= (PLAYERID << 1);
   message |= BLOCK;

   blocking = 0;
   blocknum = 0;
   }
   }
   else
   {
   blocknum = 0;
   blocksent = 0;
   }

if(message != 0)
{
msg = message;
			send(player_sock_fd,&msg,1,0);
	message = 0;
		}
	
	else{
		fprintf(stderr,"Error in comm function, 'player' or 'base' not specified\n");
		return -1;
	}
}
}
}