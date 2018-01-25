#include <pthread.h>
#define PORTNUM 8000
#define MAX_NUM_THREADS 16
#define NUM_PLAYERS 4

//definitions for players

#define NUM_ITEMS 4

//player colors
#define BLUE 0
#define RED 1
#define GREEN 2
#define YELLOW 3

//player statuses
#define NOT_CONNECTED 0
#define CONNECTED 1
#define ALIVE 2
#define DEAD 3
#define INVINCIBLE 4

//player actions
#define IDLE 0
#define TAGGING 1
#define BLOCKING 2
#define NUM_ITEMS 4
//player message constants
#define ACTOR_MASK 3
#define TARGET_MASK 12

//definitions for rounds and game
#define ROUNDSTART 1
#define ROUNDSTOP 0

#define MAX_NUM_ROUNDS 3
#define MAX_NUM_PLAYERS 4

struct player{
	int webcam_socket_fd;
	int imu_socket_fd;
	unsigned int color;
	unsigned int status;
	unsigned int hitpoints;
	unsigned int webcam_action;
	unsigned int imu_action;
	unsigned int items[NUM_ITEMS];	
	char* webcam_ip;
	char* imu_ip;
};


struct round{
	volatile int roundstatus;
	int killerpoints;
	int playerpoints;
	int timer; //placeholder for possible future timing implementations
};

//definitions for game not necesssary to use this struct but helps with organization of project
struct game{
	struct round rounds[MAX_NUM_ROUNDS];
	struct player players[MAX_NUM_PLAYERS];
};


extern struct player players[NUM_PLAYERS];
extern volatile int player_done;

//server functions
int initPlayers(pthread_t players[],int numplayers);
void playerAction(unsigned int actor, unsigned int target, unsigned int action);
