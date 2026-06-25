# DEE Examples Makefile

CC = gcc
CFLAGS = -O2 -Wall -std=c99
LDFLAGS = -lm
INC = -I..

SRCS = ../kc_hormone.c ../kc_persona.c ../kc_vision.c ../kc_active_ai.c

all: character_simulation

character_simulation: character_simulation.c
	$(CC) $(CFLAGS) $(INC) -o $@ $< $(SRCS) $(LDFLAGS)

clean:
	rm -f character_simulation

.PHONY: all clean
