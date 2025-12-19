CC = gcc
CFLAGS = -Wall -Wextra -g3 -std=c11 -Iinclude

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = output
DATA_DIR = data

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

BIN = $(BIN_DIR)/index

KEYS = ../$(DATA_DIR)/keys.txt
TEXT = ../$(DATA_DIR)/texto.txt

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) $(KEYS) $(TEXT)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
