CC = gcc
CFLAGS = -Wall -Wextra -g3 -std=c11 -Icode/include

CODE_DIR  = code
SRC_DIR   = $(CODE_DIR)/src
INC_DIR   = $(CODE_DIR)/include
BUILD_DIR = $(CODE_DIR)/build
BIN_DIR   = $(CODE_DIR)/output

BIN = $(BIN_DIR)/matrix

SRC = $(wildcard $(SRC_DIR)/*.c) matrix.c
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
