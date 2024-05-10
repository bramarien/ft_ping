CC= gcc

CFLAGS= -I. -Wall -Werror -Wextra

LIB= ft_ping.h

%.o: %.c $(LIB)
	$(CC) -c -o $@ $< $(CFLAGS)


run: 
	$(CC) -o ft_ping