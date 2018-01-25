Description of files:
base_multithread.c: Defunct multithreaded implementation for server. Sync
                    errors caused by lack of synchronization mechanisms in
                    place between two player threads. Future implementations
                    should have one player thread handle both webcam and
                    imu edison.

comm_test.c: Tests connection between two devices one client and one server
gesturecomms.c: build into app that both detects gestures and communicates
		them to main server

player_comm.c: contains all tcp/ip wrapper functions for connecting, hosting
               connections,

multithreadservertest.c: contains defunct polling implementation of server

game_server: contains data structures as well as definitions for player
             player structures
