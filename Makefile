CC=gcc
CC_FLAGS=-g -Iinclude -lX11 -lcairo -lXrender

sdewm: sdewm.c client.c window.c
	$(CC) $^ -o $@ $(CC_FLAGS)