CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -D_DEFAULT_SOURCE -I./inc

SRC = $(shell find ./ -name "*.c")
OBJ = $(patsubst %.c,%.o,$(SRC))

APP_NAME = mkuxlfs

BUILD_PATH ?= .
APP_BUILDPATH = $(BUILD_PATH)/$(APP_NAME)

all: clean $(OBJ)
	@echo BUILD PATH = $(BUILD_PATH)
	$(CC) $(CFLAGS) -o $(BUILD_PATH)/$(APP_NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ)