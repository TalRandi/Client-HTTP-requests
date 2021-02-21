all: client.c 
	 	gcc client.c -Wvla -Wall -o client
all-GDB: client.c
	 	gcc -g client.c -Wvla -Wall -o client

		 