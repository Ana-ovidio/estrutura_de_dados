CC=gcc
CFLAGS=-Wall -Wextra -g3 -std=c11 -Iinclude

SRC_DIR=src
INC_DIR=include
BUILD_DIR=build
BIN_DIR=output

SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
BIN=$(BIN_DIR)/matrix

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN)
