comm_test: comm_test.c player_comm.c player_comm.h
	gcc -g -o comm_test comm_test.c player_comm.c

#turn #include <mraa/gpio.h> into #include "mraa/gpio.h"
game_test: gameplaytest.c player_comm.c player_comm.h
	gcc -g -o game_test gameplaytest.c player_comm.c
servertest: servertest.c player_comm.c player_comm.h
	gcc -g -o testserver servertest.c player_comm.c
#Only compiles on edison turn back mraa/gpio
game_server: gameplay.c player_comm.c player_comm.h
	gcc -o gameplay gameplay.c player_comm.c -lmraai

rmout:
	rm -f  purp.txt red.txt blue.txt
