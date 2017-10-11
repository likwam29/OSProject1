# specify the compiler
CC=gcc
# specify options for the compiler
CFLAGS=-c -Wall
program: driver.o
	$(CC) driver.o -o program -lm
driver: driver.c
	$(CC) $(CFLAGS) driver.c
clean:
	rm -rf *o program
