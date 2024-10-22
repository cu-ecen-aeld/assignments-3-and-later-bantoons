#CROSS_COMPILE = aarch64-none-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall -Werror

all: aesdsocket_link

aesdsocket_link: aesdsocket_compile
	$(CC) $(CFLAGS) aesdsocket.o -o aesdsocket 

aesdsocket_compile: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o

clean:
	rm -f aesdsocket
	rm -f *.o