CC=gcc
CC_FLAGS=-Iinclude -lX11 -lcairo -lXrender
CONFIG_RELEASE=$(CC_FLAGS) -O1
CONFIG_DEBUG=$(CC_FLAGS) -g -DDEBUG

sdewm: sdewm.c client.c window.c core.c
	$(CC) $^ -o $@ $(CONFIG_RELEASE)

clean:
	rm sdewm