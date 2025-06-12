CC=gcc
CC_FLAGS=-g -Iinclude -lX11

sdewm: sdewm.c
	$(CC) $^ -o $@ $(CC_FLAGS)