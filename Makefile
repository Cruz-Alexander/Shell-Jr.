# provide a makefile with a target called shell_jr that  
# creates the executable shell_jr 

CC = gcc 
CFLAGS = -Wall -Wextra -std=c99 

all: shell_jr 

shell_jr: shell_jr.c 
	$(CC) $(CFLAGS) -o $@ $^ 

clean: 
	rm -f shell_jr 
