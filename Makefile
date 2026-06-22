CC = gcc
CFLAGS = -Werror -Wall -Wextra -pedantic -std=c23 -I./raylib-6.0_linux_amd64/include/
LDFLAGS = -L./raylib-6.0_linux_amd64/lib -l:libraylib.a

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main $(LDFLAGS)
