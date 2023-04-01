CC = cc
CFLAGS = -Wall -Wextra -std=c11 -O2

SRC = $(shell find ./ -name "*.c")
OBJ = $(patsubst %.c,%.o,$(SRC))

APP_NAME = mkvdisk
BUILD_PATH ?= .

BINARY = $(BUILD_PATH)/$(APP_NAME)

all: clean $(OBJ)
	$(CC) $(CFLAGS) -o $(BINARY) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ) $(BINARY)
