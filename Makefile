CC = gcc
CFLAGS = -g -O2 -march=native -Werror -Wall -Wextra -pedantic -std=c23 -I./raylib-6.0_linux_amd64/include/
LDFLAGS = -L./raylib-6.0_linux_amd64/lib -l:libraylib.a -lm -lX11

all: naive main

naive: src/naive.c
	$(CC) $(CFLAGS) src/naive.c -o naive $(LDFLAGS)

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main $(LDFLAGS)

.PHONY: all
