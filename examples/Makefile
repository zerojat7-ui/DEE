# DEE Examples Makefile

CC = gcc
CFLAGS = -O2 -Wall -std=c99
# include 폴더를 지정 (-I)
INC = -I../include

# 소스 파일은 src 폴더에서 가져옴
SRCS = ../src/kc_hormone.c ../src/kc_persona.c ../src/kc_vision.c ../src/kc_active_ai.c
LDFLAGS = -lm

all: character_simulation

character_simulation: character_simulation.c
	$(CC) $(CFLAGS) $(INC) -o $@ $< $(SRCS) $(LDFLAGS)

clean:
	rm -f character_simulation

.PHONY: all clean
