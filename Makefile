CC=gcc
CC_FLAGS=-g -Iinclude -lX11 -lcairo -lXrender -DDEBUG

sdewm: sdewm.c client.c window.c core.c
	$(CC) $^ -o $@ $(CC_FLAGS)

clean:
	rm sdewm