# Cave Adventure

The goal of this assignment was to create a "cave adventure" like game.

The buildrooms program would create a new directory in the current location and fill it with textfiles
named in the program.  It would then assign a "start" and "end" room and connect to other rooms.

The adventure program will allow the user to play the game. Starting in one room, the user will
type the name of connected rooms until the user has reached the end.

To compile:
gcc -o executable1 cruzst.buildrooms.c
gcc -o executable2 cruzst.adventure.c -lpthread
