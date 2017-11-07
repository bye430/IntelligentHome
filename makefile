all:servermain bt serveclient
.PHONY:all
servermain:servermain.c
	gcc servermain.c  -g -o servermain -lrt -lpthread
bt:bt.c
	gcc bt.c -g -o bt -lrt -lpthread
serveclient:serveclient.c
	gcc serveclient.c -g -o serveclient -lrt -lpthread

