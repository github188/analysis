CC = gcc
CFLAGS_RELEASE = -Wall -O2 -std=c99 -D NDEBUG
CPPFLAGS_RELEASE = -Wall -O2 -std=c99 -D NDEBUG
CFLAGS_DEBUG = -Wall -g -O0 -std=c99
CPPFLAGS_DEBUG = -Wall -g -O0 -std=c99


release: src/main.c
	$(CC) $(CFLAGS_RELEASE) src/main.c -o shfs
debug: src/main.c
	$(CC) $(CFLAGS_DEBUG) src/main.c -o shfs
