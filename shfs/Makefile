CC = gcc
CFLAGS_RELEASE = -Wall -O2 -std=c99 -D _GNU_SOURCE -D _REENTRANT -D NDEBUG
CPPFLAGS_RELEASE = -Wall -O2 -std=c99 -D _GNU_SOURCE -D _REENTRANT -D NDEBUG
CFLAGS_DEBUG = -Wall -g -O0 -std=c99 -D _GNU_SOURCE -D _REENTRANT
CPPFLAGS_DEBUG = -Wall -g -O0 -std=c99 -D _GNU_SOURCE -D _REENTRANT


release: src/main.c
	$(CC) $(CFLAGS_RELEASE) src/main.c -o shfs
debug: src/main.c
	$(CC) $(CFLAGS_DEBUG) src/main.c -o shfs
