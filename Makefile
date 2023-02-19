BIN = bin/main
CC = clang
CFLAGS = --std=c11 -fuse-ld=lld -rtlib=compiler-rt

all: $(BIN)

bin/main: main/main.c | bin
	$(CC) $(CFLAGS) $^ -o $@

bin:
	@mkdir bin
