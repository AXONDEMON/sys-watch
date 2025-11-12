CC=gcc
CFLAGS=-O2 -Wall -Wextra -pthread
INCLUDES=-Iinclude
SRC=src/syswatch.c src/metrics.c src/logs.c src/net.c src/util.c
BIN=syswatch

all: $(BIN)

$(BIN): $(SRC) include/syswatch.h
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(BIN)

clean:
	rm -f $(BIN) *.o syswatch_metrics.log