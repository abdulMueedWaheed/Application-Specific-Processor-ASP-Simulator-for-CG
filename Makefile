# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=gnu11 -Iinclude

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

# Output binary
TARGET := sim

# Find all .c files in src/
SRC := $(wildcard $(SRC_DIR)/*.c)

# Convert src/*.c → build/*.o
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# Default rule
all: $(TARGET)

# Link objects into final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each .c file into a .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Remove generated files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Run the program (default program.instr)
run: $(TARGET)
	./$(TARGET) program.instr

# Phony targets
.PHONY: all clean run
