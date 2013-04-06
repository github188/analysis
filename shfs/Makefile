CC = gcc
COMMON_FLAGS = -Wall -std=c99 -D _GNU_SOURCE -D _REENTRANT -fno-common
CFLAGS_RELEASE = ${COMMON_FLAGS} -O2 -D NDEBUG
CPPFLAGS_RELEASE = ${COMMON_FLAGS} -O2 -D NDEBUG
CFLAGS_DEBUG = ${COMMON_FLAGS} -g -O0 -rdynamic
CPPFLAGS_DEBUG = ${COMMON_FLAGS} -g -O0 -rdynamic

HEADERS = src/common.h src/string.h src/list.h
SOURCES =  src/main.c

debug: ${SOURCES} ${HEADERS}
	$(CC) $(CFLAGS_DEBUG) src/main.c -o shfs
release: ${SOURCES} $(HEADERS)
	$(CC) $(CFLAGS_RELEASE) src/main.c -o shfs
