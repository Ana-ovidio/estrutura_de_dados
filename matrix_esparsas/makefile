CC = gcc
CFLAGS = -Wall -Wextra -g3 -std=c11 -Icode/include

# Diretório raiz do projeto
CODE_DIR = code

SRC_DIR   = $(CODE_DIR)/src
INC_DIR   = $(CODE_DIR)/include
BUILD_DIR = $(CODE_DIR)/build
BIN_DIR   = $(CODE_DIR)/output

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
BIN = $(BIN_DIR)/matrix

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
	rm -rf $(BUILD_DIR) $(BIN_DIR)
